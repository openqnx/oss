// kineto (libkineto, CPU-only build): profile the way PyTorch's profiler
// does without a GPU -- initialise, prepare and start a trace of CPU ops and
// user annotations, hand kineto a CPU trace buffer of timed ops (with
// metadata) recorded on this thread, stop, and check the collected
// activities and the Chrome trace JSON it saves.
#include "common.h"
#include <kineto/libkineto.h>
#include <kineto/time_since_epoch.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <thread>
#include <unistd.h>

using namespace libkineto;
static int64_t now_ns() { return timeSinceEpoch(std::chrono::system_clock::now()); }

int main() {
  QT_BEGIN("kineto");
  libkineto_init(/*cpuOnly=*/true, /*logOnError=*/true);
  api().suppressLogMessages();
  QT_CHECK(api().isProfilerRegistered(), "profiler registered (CPU only)");
  api().initProfilerIfRegistered();
  QT_CHECK(api().isProfilerInitialized(), "profiler initialised");
  auto &prof = api().activityProfiler();
  std::set<ActivityType> types{ActivityType::CPU_OP, ActivityType::USER_ANNOTATION};
  prof.prepareTrace(types);
  prof.startTrace();
  QT_CHECK(prof.isActive(), "trace active after startTrace");

  auto buf = std::make_unique<CpuTraceBuffer>();
  int64_t t0 = now_ns();
  buf->span = TraceSpan(t0, t0, "qnx-iteration");
  buf->gpuOpCount = -1;
  const char *ops[] = { "aten::matmul", "aten::add", "aten::relu" };
  for (int i = 0; i < 3; i++) {
    int64_t s = now_ns();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    buf->emplace_activity(buf->span, ActivityType::CPU_OP, ops[i]);
    auto &a = *buf->activities.back();
    a.startTime = s; a.endTime = now_ns(); a.id = 100 + i;
    a.device = (int32_t)getpid(); a.resource = (int32_t)systemThreadId(); a.threadId = (int32_t)threadId();
    a.addMetadataQuoted("Input Dims", "[[2, 2], [2, 2]]");
  }
  buf->emplace_activity(buf->span, ActivityType::USER_ANNOTATION, "qnx::step");
  { auto &u = *buf->activities.back(); u.startTime = t0; u.endTime = now_ns(); u.id = 200;
    u.device = (int32_t)getpid(); u.resource = (int32_t)systemThreadId(); }
  buf->span.endTime = now_ns();
  prof.transferCpuTrace(std::move(buf));
  auto trace = prof.stopTrace();
  QT_CHECK(trace != nullptr, "stopTrace returned a trace");
  if (!trace) QT_END();
  auto acts = trace->activities();
  int cpu = 0, user = 0, seen = 0; bool timed = true;
  for (auto *a : *acts) {
    if (a->type() == ActivityType::CPU_OP) { cpu++; for (auto *o : ops) seen += a->name() == o; timed &= a->duration() >= 1000000; }
    if (a->type() == ActivityType::USER_ANNOTATION && a->name() == "qnx::step") user++;
  }
  printf("  info %zu activities: %d CPU ops, %d annotations\n", acts ? acts->size() : 0, cpu, user);
  QT_CHECK(cpu == 3 && seen == 3, "the 3 CPU ops, by name");
  QT_CHECK(timed, "each op lasted >= 1 ms (the sleeps)");
  QT_CHECK(user == 1, "the user annotation");
  const char *tmp = getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp";
  char path[256]; snprintf(path, sizeof path, "%s/qnxport-kineto-%d.json", tmp, (int)getpid());
  trace->save(path);
  std::ifstream f(path); std::stringstream ss; ss << f.rdbuf(); std::string js = ss.str();
  printf("  info %s: %zu bytes\n", path, js.size());
  QT_CHECK(js.find("\"traceEvents\"") != std::string::npos, "Chrome trace JSON saved (traceEvents)");
  QT_CHECK(js.find("aten::matmul") != std::string::npos && js.find("qnx::step") != std::string::npos, "with the ops and the annotation");
  QT_CHECK(js.find("Input Dims") != std::string::npos, "and the op metadata");
  unlink(path);
  QT_END();
}
