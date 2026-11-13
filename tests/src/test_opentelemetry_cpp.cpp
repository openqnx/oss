// opentelemetry-cpp: the tracing and metrics SDK end to end, with exporters
// whose output can be inspected.
//   - traces to the in-memory exporter: span names, export order, one trace
//     id for parent and child, the child's parent span id, attributes, an
//     event, the status, span kind, the resource and the instrumentation scope,
//   - W3C trace context: a traceparent header injected and extracted again,
//   - metrics to the in-memory exporter: a counter summed per attribute set
//     and a histogram's count/sum/min/max,
//   - the ostream exporter: a span printed to a string stream,
//   - OTLP/HTTP: a span exported to a small HTTP listener on loopback inside
//     this test -- POST /v1/traces, content type application/x-protobuf, and
//     a protobuf body that carries the span and service names,
//   - OTLP/gRPC: a span exported to a gRPC TraceService on loopback inside
//     this test, decoded there (span name, attribute, service name).
#include "common.h"

#include <opentelemetry/context/propagation/text_map_propagator.h>
#include <opentelemetry/context/runtime_context.h>
#include <opentelemetry/exporters/memory/in_memory_metric_data.h>
#include <opentelemetry/exporters/memory/in_memory_metric_exporter_factory.h>
#include <opentelemetry/exporters/memory/in_memory_span_data.h>
#include <opentelemetry/exporters/memory/in_memory_span_exporter_factory.h>
#include <opentelemetry/exporters/ostream/span_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_grpc_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_grpc_exporter_options.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter_options.h>
#include <opentelemetry/metrics/meter.h>
#include <opentelemetry/metrics/sync_instruments.h>
#include <opentelemetry/sdk/metrics/data/point_data.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_options.h>
#include <opentelemetry/sdk/metrics/meter_provider.h>
#include <opentelemetry/sdk/metrics/meter_provider_factory.h>
#include <opentelemetry/sdk/resource/resource.h>
#include <opentelemetry/sdk/trace/simple_processor_factory.h>
#include <opentelemetry/sdk/trace/span_data.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/trace/propagation/http_trace_context.h>
#include <opentelemetry/trace/scope.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/tracer.h>

// The generated OTLP classes carry an export macro, OPENTELEMETRY_PROTO_API,
// that the consumer defines (empty here: see opentelemetry_proto.pc); the two
// headers around them silence warnings in generated code.
#include <opentelemetry/exporters/otlp/protobuf_include_prefix.h>
#include <opentelemetry/proto/collector/trace/v1/trace_service.grpc.pb.h>
#include <opentelemetry/exporters/otlp/protobuf_include_suffix.h>
#include <grpcpp/grpcpp.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstdlib>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace api = opentelemetry;
namespace sdktrace = opentelemetry::sdk::trace;
namespace sdkmetrics = opentelemetry::sdk::metrics;
namespace sdkres = opentelemetry::sdk::resource;
namespace memory = opentelemetry::exporter::memory;
namespace otlp = opentelemetry::exporter::otlp;
namespace nostd = opentelemetry::nostd;

static std::string hex(const api::trace::TraceId &id) {
  char b[32];
  id.ToLowerBase16(b);
  return std::string(b, 32);
}
static std::string hex(const api::trace::SpanId &id) {
  char b[16];
  id.ToLowerBase16(b);
  return std::string(b, 16);
}

static sdkres::Resource test_resource() {
  return sdkres::Resource::Create({{"service.name", std::string("qnx-otel-test")}});
}

// One span through `exporter`, flushed and shut down.
static bool export_one_span(std::unique_ptr<sdktrace::SpanExporter> exporter, const char *name) {
  auto processor = sdktrace::SimpleSpanProcessorFactory::Create(std::move(exporter));
  auto provider = sdktrace::TracerProviderFactory::Create(std::move(processor), test_resource());
  auto tracer = provider->GetTracer("qnx.otlp", "1.0.0");
  auto span = tracer->StartSpan(name);
  span->SetAttribute("answer", static_cast<int64_t>(42));
  span->End();
  bool ok = provider->ForceFlush(std::chrono::seconds(10));
  provider->Shutdown(std::chrono::seconds(10));
  return ok;
}

// ---- carrier for the W3C propagator -----------------------------------------
class MapCarrier : public api::context::propagation::TextMapCarrier {
public:
  nostd::string_view Get(nostd::string_view key) const noexcept override {
    auto it = headers.find(std::string(key));
    return it == headers.end() ? nostd::string_view() : nostd::string_view(it->second);
  }
  void Set(nostd::string_view key, nostd::string_view value) noexcept override {
    headers[std::string(key)] = std::string(value);
  }
  std::map<std::string, std::string> headers;
};

// ---- a one-request HTTP listener on loopback --------------------------------
struct HttpCapture {
  int listen_fd = -1;
  int port = 0;
  std::string request_line, headers, body;
  bool complete = false;
};

static bool wait_readable(int fd, int ms) {
  struct pollfd p = {fd, POLLIN, 0};
  return poll(&p, 1, ms) == 1 && (p.revents & POLLIN);
}

static bool http_listen(HttpCapture &c) {
  c.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (c.listen_fd < 0) return false;
  struct sockaddr_in a;
  memset(&a, 0, sizeof a);
  a.sin_family = AF_INET;
  a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  a.sin_port = 0;  // any free port
  socklen_t len = sizeof a;
  if (bind(c.listen_fd, (struct sockaddr *)&a, sizeof a) != 0 || listen(c.listen_fd, 4) != 0 ||
      getsockname(c.listen_fd, (struct sockaddr *)&a, &len) != 0)
    return false;
  c.port = ntohs(a.sin_port);
  return true;
}

static std::string lower(std::string s) {
  for (auto &ch : s) ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
  return s;
}

// Accept one connection, read one request (headers + Content-Length body),
// answer 200.  Every wait is bounded.
static void http_serve_once(HttpCapture *c) {
  if (!wait_readable(c->listen_fd, 10000)) return;
  int fd = accept(c->listen_fd, nullptr, nullptr);
  if (fd < 0) return;
  std::string data;
  size_t header_end = std::string::npos, want = 0;
  char buf[4096];
  for (;;) {
    if (header_end != std::string::npos && data.size() >= header_end + 4 + want) break;
    if (!wait_readable(fd, 10000)) break;
    ssize_t n = recv(fd, buf, sizeof buf, 0);
    if (n <= 0) break;
    data.append(buf, static_cast<size_t>(n));
    if (header_end == std::string::npos) {
      header_end = data.find("\r\n\r\n");
      if (header_end != std::string::npos) {
        std::string h = lower(data.substr(0, header_end));
        size_t p = h.find("content-length:");
        if (p != std::string::npos) want = static_cast<size_t>(atol(h.c_str() + p + 15));
        if (h.find("expect: 100-continue") != std::string::npos) {
          static const char cont[] = "HTTP/1.1 100 Continue\r\n\r\n";
          (void)send(fd, cont, sizeof cont - 1, 0);
        }
      }
    }
  }
  if (header_end != std::string::npos && data.size() >= header_end + 4 + want) {
    size_t eol = data.find("\r\n");
    c->request_line = data.substr(0, eol);
    c->headers = lower(data.substr(eol + 2, header_end - eol - 2));
    c->body = data.substr(header_end + 4, want);
    c->complete = true;
    static const char ok[] =
        "HTTP/1.1 200 OK\r\nContent-Type: application/x-protobuf\r\nContent-Length: 0\r\n"
        "Connection: close\r\n\r\n";
    (void)send(fd, ok, sizeof ok - 1, 0);
  }
  shutdown(fd, SHUT_RDWR);
  close(fd);
}

// ---- a gRPC OTLP trace collector on loopback --------------------------------
namespace collector = opentelemetry::proto::collector::trace::v1;
class TraceCollector final : public collector::TraceService::Service {
public:
  grpc::Status Export(grpc::ServerContext *, const collector::ExportTraceServiceRequest *req,
                      collector::ExportTraceServiceResponse *) override {
    std::lock_guard<std::mutex> g(mu);
    ++calls;
    for (const auto &rs : req->resource_spans()) {
      for (const auto &kv : rs.resource().attributes())
        if (kv.key() == "service.name") service = kv.value().string_value();
      for (const auto &ss : rs.scope_spans()) {
        scope = ss.scope().name();
        for (const auto &s : ss.spans()) {
          span_name = s.name();
          trace_id_len = s.trace_id().size();
          for (const auto &kv : s.attributes())
            if (kv.key() == "answer") answer = kv.value().int_value();
        }
      }
    }
    return grpc::Status::OK;
  }
  std::mutex mu;
  int calls = 0;
  std::string service, scope, span_name;
  size_t trace_id_len = 0;
  long long answer = 0;
};

int main() {
  QT_BEGIN("opentelemetry-cpp");
  // The exporters must reach the loopback listeners directly.
  unsetenv("http_proxy"); unsetenv("HTTP_PROXY"); unsetenv("https_proxy"); unsetenv("HTTPS_PROXY");
  unsetenv("all_proxy"); unsetenv("ALL_PROXY"); unsetenv("grpc_proxy");
  setenv("no_proxy", "127.0.0.1,localhost", 1);

  // ---- traces: in-memory exporter -------------------------------------------
  {
    std::shared_ptr<memory::InMemorySpanData> data;
    auto exporter = memory::InMemorySpanExporterFactory::Create(data);
    auto processor = sdktrace::SimpleSpanProcessorFactory::Create(std::move(exporter));
    auto provider = sdktrace::TracerProviderFactory::Create(std::move(processor), test_resource());
    auto tracer = provider->GetTracer("qnx.tracer", "2.1.0");

    std::string parent_trace, parent_span;
    MapCarrier carrier;
    {
      api::trace::StartSpanOptions opts;
      opts.kind = api::trace::SpanKind::kServer;
      auto parent = tracer->StartSpan("parent-op", {{"http.route", "/items"}}, opts);
      auto scope = tracer->WithActiveSpan(parent);
      parent_trace = hex(parent->GetContext().trace_id());
      parent_span = hex(parent->GetContext().span_id());
      QT_CHECK(parent->GetContext().IsValid() && parent->IsRecording(), "span context valid, recording");

      // W3C trace context from the active span.
      api::trace::propagation::HttpTraceContext propagator;
      propagator.Inject(carrier, api::context::RuntimeContext::GetCurrent());

      {
        auto child = tracer->StartSpan("child-op");  // parent: the active span
        child->SetAttribute("items", static_cast<int64_t>(7));
        child->SetAttribute("ratio", 0.25);
        child->SetAttribute("cached", true);
        child->AddEvent("lookup", {{"key", "abc"}});
        child->SetStatus(api::trace::StatusCode::kError, "not found");
        child->End();
      }
      parent->End();
    }

    auto spans = data->GetSpans();
    QT_CHECK(spans.size() == 2, "two spans exported");
    if (spans.size() == 2) {
      const auto &child = *spans[0], &parent = *spans[1];
      QT_CHECK(std::string(child.GetName()) == "child-op" && std::string(parent.GetName()) == "parent-op",
               "span names, child ended first");
      QT_CHECK(hex(child.GetTraceId()) == parent_trace && hex(parent.GetTraceId()) == parent_trace,
               "parent and child share the trace id");
      QT_CHECK(hex(parent.GetSpanId()) == parent_span && hex(child.GetParentSpanId()) == parent_span,
               "child's parent span id is the parent's span id");
      QT_CHECK(hex(child.GetSpanId()) != parent_span && child.GetSpanId().IsValid(), "child has its own span id");
      QT_CHECK(!parent.GetParentSpanId().IsValid(), "root span has no parent");
      QT_CHECK(parent.GetSpanKind() == api::trace::SpanKind::kServer &&
               child.GetSpanKind() == api::trace::SpanKind::kInternal, "span kinds");

      const auto &ca = child.GetAttributes();
      auto items = ca.find("items");
      auto ratio = ca.find("ratio");
      auto cached = ca.find("cached");
      QT_CHECK(ca.size() == 3 && items != ca.end() && nostd::holds_alternative<int64_t>(items->second) &&
               nostd::get<int64_t>(items->second) == 7, "int attribute");
      QT_CHECK(ratio != ca.end() && nostd::holds_alternative<double>(ratio->second) &&
               nostd::get<double>(ratio->second) == 0.25, "double attribute");
      QT_CHECK(cached != ca.end() && nostd::holds_alternative<bool>(cached->second) &&
               nostd::get<bool>(cached->second), "bool attribute");
      const auto &pa = parent.GetAttributes();
      auto route = pa.find("http.route");
      QT_CHECK(route != pa.end() && nostd::holds_alternative<std::string>(route->second) &&
               nostd::get<std::string>(route->second) == "/items", "string attribute given at start");

      QT_CHECK(child.GetEvents().size() == 1 && child.GetEvents()[0].GetName() == "lookup" &&
               child.GetEvents()[0].GetAttributes().count("key") == 1, "event with attribute");
      QT_CHECK(child.GetStatus() == api::trace::StatusCode::kError &&
               std::string(child.GetDescription()) == "not found", "status and description");
      QT_CHECK(parent.GetStatus() == api::trace::StatusCode::kUnset, "status unset by default");
      QT_CHECK(child.GetDuration().count() >= 0 && parent.GetDuration() >= child.GetDuration(),
               "parent lasts at least as long as the child");

      const auto &ra = parent.GetResource().GetAttributes();
      auto svc = ra.find("service.name");
      QT_CHECK(svc != ra.end() && nostd::get<std::string>(svc->second) == "qnx-otel-test", "resource service.name");
      QT_CHECK(parent.GetInstrumentationScope().GetName() == "qnx.tracer" &&
               parent.GetInstrumentationScope().GetVersion() == "2.1.0", "instrumentation scope");
    }

    // traceparent: 00-<trace id>-<span id>-<flags>, and back.  The flags
    // carry the sampled bit (01) and, since trace context level 2, the
    // random-trace-id bit (02).
    const std::string tp = carrier.headers["traceparent"];
    const std::string tp_head = "00-" + parent_trace + "-" + parent_span + "-";
    QT_CHECK(tp.size() == tp_head.size() + 2 && tp.compare(0, tp_head.size(), tp_head) == 0,
             "traceparent header injected: version, trace id, span id");
    QT_CHECK(tp.size() == 55 && (strtol(tp.c_str() + 53, nullptr, 16) & 1) == 1,
             "traceparent header: sampled flag set");
    {
      api::trace::propagation::HttpTraceContext propagator;
      api::context::Context empty;
      auto ctx = propagator.Extract(carrier, empty);
      auto remote = api::trace::GetSpan(ctx)->GetContext();
      QT_CHECK(remote.IsValid() && remote.IsRemote() && remote.IsSampled() &&
               hex(remote.trace_id()) == parent_trace && hex(remote.span_id()) == parent_span,
               "traceparent header extracted");
    }
    QT_CHECK(provider->Shutdown(std::chrono::seconds(10)), "tracer provider shut down");
  }

  // ---- metrics: in-memory exporter ------------------------------------------
  {
    auto data = std::make_shared<memory::SimpleAggregateInMemoryMetricData>();
    auto exporter = memory::InMemoryMetricExporterFactory::Create(data);
    sdkmetrics::PeriodicExportingMetricReaderOptions ro;
    // Never on the timer within this test: collection is forced below.
    ro.export_interval_millis = std::chrono::milliseconds(3600 * 1000);
    ro.export_timeout_millis = std::chrono::milliseconds(10 * 1000);
    std::shared_ptr<sdkmetrics::MetricReader> reader =
        sdkmetrics::PeriodicExportingMetricReaderFactory::Create(std::move(exporter), ro);
    auto provider = sdkmetrics::MeterProviderFactory::Create();
    provider->AddMetricReader(reader);
    auto meter = provider->GetMeter("qnx.meter", "1.0.0");

    auto counter = meter->CreateUInt64Counter("requests", "handled requests", "1");
    counter->Add(3, {{"route", "/a"}});
    counter->Add(4, {{"route", "/a"}});
    counter->Add(5, {{"route", "/b"}});
    auto histogram = meter->CreateDoubleHistogram("latency", "request latency", "ms");
    for (double v : {2.0, 8.0, 20.0, 70.0}) histogram->Record(v, api::context::Context{});

    QT_CHECK(provider->ForceFlush(std::chrono::seconds(10)), "metrics flushed");

    const auto &points = data->Get("qnx.meter", "requests");
    long long a = -1, b = -1;
    for (const auto &p : points) {
      auto it = p.first.find("route");
      if (it == p.first.end() || !nostd::holds_alternative<sdkmetrics::SumPointData>(p.second)) continue;
      const auto &sum = nostd::get<sdkmetrics::SumPointData>(p.second);
      const long long v = nostd::holds_alternative<int64_t>(sum.value_)
                              ? static_cast<long long>(nostd::get<int64_t>(sum.value_))
                              : static_cast<long long>(nostd::get<double>(sum.value_));
      const std::string r = nostd::get<std::string>(it->second);
      if (r == "/a") a = v;
      if (r == "/b") b = v;
    }
    QT_CHECK(points.size() == 2, "counter: one point per attribute set");
    QT_CHECK(a == 7 && b == 5, "counter: sums 3+4 = 7 and 5");

    const auto &hp = data->Get("qnx.meter", "latency");
    bool hist_ok = false;
    if (hp.size() == 1 && nostd::holds_alternative<sdkmetrics::HistogramPointData>(hp.begin()->second)) {
      const auto &h = nostd::get<sdkmetrics::HistogramPointData>(hp.begin()->second);
      uint64_t in_buckets = 0;
      for (auto c : h.counts_) in_buckets += c;
      hist_ok = h.count_ == 4 && nostd::get<double>(h.sum_) == 100.0 && nostd::get<double>(h.min_) == 2.0 &&
                nostd::get<double>(h.max_) == 70.0 && in_buckets == 4 &&
                h.counts_.size() == h.boundaries_.size() + 1;
    }
    QT_CHECK(hist_ok, "histogram: count 4, sum 100, min 2, max 70, buckets");
    QT_CHECK(provider->Shutdown(std::chrono::seconds(10)), "meter provider shut down");
  }

  // ---- ostream exporter -----------------------------------------------------
  {
    std::ostringstream out;
    bool ok = export_one_span(opentelemetry::exporter::trace::OStreamSpanExporterFactory::Create(out),
                              "printed-span");
    const std::string s = out.str();
    QT_CHECK(ok && s.find("name          : printed-span") != std::string::npos, "ostream: span name printed");
    QT_CHECK(s.find("answer: 42") != std::string::npos && s.find("qnx-otel-test") != std::string::npos,
             "ostream: attribute and resource printed");
  }

  // ---- OTLP over HTTP to a loopback listener --------------------------------
  {
    HttpCapture cap;
    const bool listening = http_listen(cap);
    QT_CHECK(listening, "OTLP/HTTP: loopback listener");
    if (listening) {
      std::thread server(http_serve_once, &cap);
      otlp::OtlpHttpExporterOptions opts;
      opts.url = "http://127.0.0.1:" + std::to_string(cap.port) + "/v1/traces";
      opts.content_type = otlp::HttpRequestContentType::kBinary;
      opts.timeout = std::chrono::seconds(10);
      const bool ok = export_one_span(otlp::OtlpHttpExporterFactory::Create(opts), "otlp-http-span");
      server.join();
      QT_CHECK(ok && cap.complete, "OTLP/HTTP: request received");
      QT_CHECK(cap.request_line.compare(0, 16, "POST /v1/traces ") == 0, "OTLP/HTTP: POST /v1/traces");
      QT_CHECK(cap.headers.find("content-type: application/x-protobuf") != std::string::npos,
               "OTLP/HTTP: content type application/x-protobuf");
      collector::ExportTraceServiceRequest req;
      const bool parsed = !cap.body.empty() && req.ParseFromString(cap.body);
      QT_CHECK(parsed, "OTLP/HTTP: body is an ExportTraceServiceRequest");
      bool span_ok = false, svc_ok = false;
      if (parsed && req.resource_spans_size() == 1) {
        const auto &rs = req.resource_spans(0);
        for (const auto &kv : rs.resource().attributes())
          if (kv.key() == "service.name" && kv.value().string_value() == "qnx-otel-test") svc_ok = true;
        if (rs.scope_spans_size() == 1 && rs.scope_spans(0).spans_size() == 1) {
          const auto &s = rs.scope_spans(0).spans(0);
          span_ok = s.name() == "otlp-http-span" && s.trace_id().size() == 16 && s.span_id().size() == 8 &&
                    rs.scope_spans(0).scope().name() == "qnx.otlp";
        }
      }
      QT_CHECK(span_ok, "OTLP/HTTP: span name, ids and scope in the body");
      QT_CHECK(svc_ok, "OTLP/HTTP: resource service.name in the body");
    }
    if (cap.listen_fd >= 0) close(cap.listen_fd);
  }

  // ---- OTLP over gRPC to a loopback collector -------------------------------
  {
    TraceCollector service;
    int port = 0;
    grpc::ServerBuilder builder;
    builder.AddListeningPort("127.0.0.1:0", grpc::InsecureServerCredentials(), &port);
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server = builder.BuildAndStart();
    QT_CHECK(server && port > 0, "OTLP/gRPC: loopback collector");
    if (server && port > 0) {
      otlp::OtlpGrpcExporterOptions opts;
      opts.endpoint = "http://127.0.0.1:" + std::to_string(port);
      opts.use_ssl_credentials = false;
      opts.timeout = std::chrono::seconds(10);
      const bool ok = export_one_span(otlp::OtlpGrpcExporterFactory::Create(opts), "otlp-grpc-span");
      server->Shutdown(std::chrono::system_clock::now() + std::chrono::seconds(5));
      server->Wait();
      std::lock_guard<std::mutex> g(service.mu);
      QT_CHECK(ok && service.calls == 1, "OTLP/gRPC: one Export call received");
      QT_CHECK(service.span_name == "otlp-grpc-span" && service.trace_id_len == 16 && service.answer == 42,
               "OTLP/gRPC: span name, trace id and attribute");
      QT_CHECK(service.service == "qnx-otel-test" && service.scope == "qnx.otlp",
               "OTLP/gRPC: resource and scope");
    }
  }

  QT_END();
}
