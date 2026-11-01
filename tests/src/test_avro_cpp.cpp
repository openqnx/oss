// Apache Avro C++: a record schema (string, int, long, double, boolean,
// array, map, union, enum, bytes) compiled from JSON; a generic datum
// encoded to binary with the bytes checked against the Avro spec's
// zig-zag/varint encoding, decoded back and compared; the JSON encoding of
// the same record; schema resolution reading old data with a newer schema
// (a field added with a default); object container files written and read
// with the null, deflate, snappy and zstd codecs; and an invalid schema
// rejected with avro::Exception; avrogencpp generating C++ from the schema.
#include "common.h"
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <unistd.h>
// The codecs the library was built with (avro-cpp's CMakeLists.txt defines
// these for its own compilation only; DataFile.hh's Codec enum needs them).
#define SNAPPY_CODEC_AVAILABLE 1
#define ZSTD_CODEC_AVAILABLE 1
#include <avro/Compiler.hh>
#include <avro/DataFile.hh>
#include <avro/Decoder.hh>
#include <avro/Encoder.hh>
#include <avro/Generic.hh>
#include <avro/GenericDatum.hh>
#include <avro/Stream.hh>
#include <avro/ValidSchema.hh>

static const char *SCHEMA = R"({
  "type": "record", "name": "Reading", "namespace": "qnx.test",
  "fields": [
    {"name": "sensor", "type": "string"},
    {"name": "count",  "type": "int"},
    {"name": "stamp",  "type": "long"},
    {"name": "value",  "type": "double"},
    {"name": "ok",     "type": "boolean"},
    {"name": "samples","type": {"type": "array", "items": "int"}},
    {"name": "tags",   "type": {"type": "map", "values": "string"}},
    {"name": "note",   "type": ["null", "string"]},
    {"name": "level",  "type": {"type": "enum", "name": "Level", "symbols": ["LOW", "MID", "HIGH"]}},
    {"name": "raw",    "type": "bytes"}
  ]})";

static avro::GenericDatum make(const avro::ValidSchema &s, int i) {
  avro::GenericDatum d(s);
  auto &r = d.value<avro::GenericRecord>();
  r.field("sensor").value<std::string>() = "temp-" + std::to_string(i);
  r.field("count").value<int32_t>() = i;
  r.field("stamp").value<int64_t>() = 1700000000000LL + i;
  r.field("value").value<double>() = 20.5 + i / 4.0;
  r.field("ok").value<bool>() = (i % 2) == 0;
  auto &arr = r.field("samples").value<avro::GenericArray>().value();
  for (int k = 0; k < 3; k++) arr.emplace_back(int32_t(i * 10 + k));
  auto &m = r.field("tags").value<avro::GenericMap>().value();
  m.emplace_back("site", avro::GenericDatum(std::string("lab")));
  auto &note = r.field("note");
  if (i % 3 == 0) { note.selectBranch(1); note.value<std::string>() = "every third"; }
  else note.selectBranch(0);
  r.field("level").value<avro::GenericEnum>().set(i % 3);
  r.field("raw").value<std::vector<uint8_t>>() = { 0x00, 0xff, uint8_t(i) };
  return d;
}
static bool same(const avro::GenericDatum &a, const avro::GenericDatum &b) {
  const auto &x = a.value<avro::GenericRecord>(), &y = b.value<avro::GenericRecord>();
  bool ok = x.field("sensor").value<std::string>() == y.field("sensor").value<std::string>()
         && x.field("count").value<int32_t>() == y.field("count").value<int32_t>()
         && x.field("stamp").value<int64_t>() == y.field("stamp").value<int64_t>()
         && x.field("value").value<double>() == y.field("value").value<double>()
         && x.field("ok").value<bool>() == y.field("ok").value<bool>()
         && x.field("level").value<avro::GenericEnum>().symbol() == y.field("level").value<avro::GenericEnum>().symbol()
         && x.field("raw").value<std::vector<uint8_t>>() == y.field("raw").value<std::vector<uint8_t>>()
         && x.field("note").unionBranch() == y.field("note").unionBranch();
  const auto &ax = x.field("samples").value<avro::GenericArray>().value(), &ay = y.field("samples").value<avro::GenericArray>().value();
  ok = ok && ax.size() == ay.size();
  for (size_t k = 0; ok && k < ax.size(); k++) ok = ax[k].value<int32_t>() == ay[k].value<int32_t>();
  const auto &mx = x.field("tags").value<avro::GenericMap>().value(), &my = y.field("tags").value<avro::GenericMap>().value();
  ok = ok && mx.size() == my.size() && mx[0].first == my[0].first && mx[0].second.value<std::string>() == my[0].second.value<std::string>();
  return ok;
}
static std::vector<uint8_t> encode(const avro::GenericDatum &d) {
  auto out = avro::memoryOutputStream();
  auto e = avro::binaryEncoder();
  e->init(*out);
  avro::encode(*e, d);
  e->flush();
  auto in = avro::memoryInputStream(*out);
  avro::StreamReader rd(*in);
  std::vector<uint8_t> bytes;
  while (rd.hasMore()) bytes.push_back(rd.read());
  return bytes;
}

int main() {
  QT_BEGIN("avro-cpp");
  try {
    avro::ValidSchema s = avro::compileJsonSchemaFromString(SCHEMA);
    QT_CHECK(s.root()->type() == avro::AVRO_RECORD && s.root()->leaves() == 10, "record schema with 10 fields compiled");
    QT_CHECK(s.root()->name().fullname() == "qnx.test.Reading", "full name qnx.test.Reading");

    // the Avro spec's encodings: int 1 -> 02, long -64 -> 7f, string "ab" -> 04 61 62
    {
      avro::ValidSchema li = avro::compileJsonSchemaFromString(R"({"type":"record","name":"P","fields":[
        {"name":"a","type":"int"},{"name":"b","type":"long"},{"name":"c","type":"string"}]})");
      avro::GenericDatum d(li);
      auto &r = d.value<avro::GenericRecord>();
      r.field("a").value<int32_t>() = 1; r.field("b").value<int64_t>() = -64; r.field("c").value<std::string>() = "ab";
      std::vector<uint8_t> b = encode(d);
      QT_CHECK((b == std::vector<uint8_t>{ 0x02, 0x7f, 0x04, 'a', 'b' }), "binary encoding matches the spec (02 7f 04 61 62)");
    }

    // binary round trip
    avro::GenericDatum d = make(s, 3);
    std::vector<uint8_t> bytes = encode(d);
    printf("  info record encodes to %zu bytes\n", bytes.size());
    {
      auto in = avro::memoryInputStream(bytes.data(), bytes.size());
      auto dec = avro::binaryDecoder(); dec->init(*in);
      avro::GenericDatum back(s);
      avro::decode(*dec, back);
      QT_CHECK(same(d, back), "binary round trip of a generic record");
      QT_CHECK(back.value<avro::GenericRecord>().field("note").value<std::string>() == "every third", "union branch string kept");
    }

    // JSON encoding
    {
      auto out = avro::memoryOutputStream();
      auto e = avro::jsonEncoder(s); e->init(*out); avro::encode(*e, d); e->flush();
      auto in = avro::memoryInputStream(*out);
      avro::StreamReader rd(*in); std::string js;
      while (rd.hasMore()) js.push_back(char(rd.read()));
      printf("  info JSON: %s\n", js.c_str());
      QT_CHECK(js.find("\"sensor\":\"temp-3\"") != std::string::npos && js.find("\"level\":\"LOW\"") != std::string::npos &&
               js.find("\"note\":{\"string\":\"every third\"}") != std::string::npos, "JSON encoding (union as {\"string\": ...})");
      auto in2 = avro::memoryInputStream(reinterpret_cast<const uint8_t *>(js.data()), js.size());
      auto jd = avro::jsonDecoder(s); jd->init(*in2);
      avro::GenericDatum back(s); avro::decode(*jd, back);
      QT_CHECK(same(d, back), "JSON decode gives the same record");
    }

    // schema resolution: written with v1, read with v2 (new field with a default)
    {
      avro::ValidSchema v1 = avro::compileJsonSchemaFromString(R"({"type":"record","name":"V","fields":[{"name":"x","type":"int"}]})");
      avro::ValidSchema v2 = avro::compileJsonSchemaFromString(R"({"type":"record","name":"V","fields":[
        {"name":"x","type":"long"},{"name":"unit","type":"string","default":"celsius"}]})");
      avro::GenericDatum w(v1); w.value<avro::GenericRecord>().field("x").value<int32_t>() = 77;
      std::vector<uint8_t> b = encode(w);
      auto in = avro::memoryInputStream(b.data(), b.size());
      auto rdec = avro::resolvingDecoder(v1, v2, avro::binaryDecoder()); rdec->init(*in);
      avro::GenericDatum r(v2); avro::decode(*rdec, r);
      auto &rr = r.value<avro::GenericRecord>();
      QT_CHECK(rr.field("x").value<int64_t>() == 77 && rr.field("unit").value<std::string>() == "celsius",
               "resolving decoder: int promoted to long, default filled in");
    }

    // object container files, every codec
    struct { avro::Codec c; const char *name; } codecs[] = {
      { avro::NULL_CODEC, "null" }, { avro::DEFLATE_CODEC, "deflate" },
      { avro::SNAPPY_CODEC, "snappy" }, { avro::ZSTD_CODEC, "zstd" } };
    for (auto &c : codecs) {
      char path[128]; snprintf(path, sizeof path, "/tmp/qnxport-avro-%d-%s.avro", (int)getpid(), c.name);
      const int N = 500;
      bool ok = true; int n = 0;
      try {
        {
          avro::DataFileWriter<avro::GenericDatum> w(path, s, 4096, c.c);
          for (int i = 0; i < N; i++) w.write(make(s, i));
          w.close();
        }
        avro::DataFileReader<avro::GenericDatum> r(path);
        ok = r.dataSchema().toJson(false) == s.toJson(false);
        avro::GenericDatum got(r.dataSchema());
        while (r.read(got)) { if (!same(got, make(s, n))) ok = false; n++; }
        r.close();
      } catch (const std::exception &e) { printf("  info %s: %s\n", c.name, e.what()); ok = false; }
      FILE *f = fopen(path, "rb"); long size = 0;
      if (f) { fseek(f, 0, SEEK_END); size = ftell(f); fclose(f); }
      unlink(path);
      printf("  info %s container file: %ld bytes for %d records\n", c.name, size, n);
      char what[96]; snprintf(what, sizeof what, "container file with the %s codec: %d records read back intact", c.name, N);
      QT_CHECK(ok && n == N, what);
    }

    // avrogencpp, the code generator, run on the target
    {
      char sch[128], hh[128], cmd[512];
      snprintf(sch, sizeof sch, "/tmp/qnxport-avro-%d.json", (int)getpid());
      snprintf(hh, sizeof hh, "/tmp/qnxport-avro-%d.hh", (int)getpid());
      FILE *f = fopen(sch, "w"); fputs(SCHEMA, f); fclose(f);
      snprintf(cmd, sizeof cmd, "{ /usr/local/bin/avrogencpp -i %s -o %s -n gen || avrogencpp -i %s -o %s -n gen; } >/dev/null 2>&1", sch, hh, sch, hh);
      int rc = system(cmd);
      std::string h; f = fopen(hh, "r");
      if (f) { char b[4096]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) h.append(b, n); fclose(f); }
      unlink(sch); unlink(hh);
      QT_CHECK(rc == 0 && h.find("namespace gen") != std::string::npos && h.find("struct Reading") != std::string::npos &&
               h.find("codec_traits<gen::Reading>") != std::string::npos, "avrogencpp generates a C++ struct and codec_traits");
    }

    bool threw = false;
    try { avro::compileJsonSchemaFromString(R"({"type":"record","name":"Bad","fields":[{"name":"x","type":"nosuchtype"}]})"); }
    catch (const avro::Exception &e) { threw = true; printf("  info rejected: %s\n", e.what()); }
    QT_CHECK(threw, "unknown type in a schema: avro::Exception");
  } catch (const std::exception &e) {
    printf("  info exception: %s\n", e.what());
    QT_CHECK(false, "no exception");
  }
  QT_END();
}
