#include "common.h"
#define MCAP_IMPLEMENTATION
#include <mcap/reader.hpp>
#include <mcap/writer.hpp>
#include <cstdio>
#include <string>
#include <unistd.h>
/* mcap-cpp: write an MCAP file -- one schema, two channels, 1000 messages
 * in zstd-compressed and in lz4-compressed chunks -- and read it back:
 * every message, in order, on its topic, with its payload. */
static int roundtrip(mcap::Compression c, const char *name) {
  char path[64]; snprintf(path, sizeof path, "/tmp/mcap-%d-%s.mcap", (int)getpid(), name);
  mcap::McapWriter w; mcap::McapWriterOptions o("ros2"); o.compression = c; o.chunkSize = 4096;
  if (!w.open(path, o).ok()) return -1;
  mcap::Schema sc("std_msgs/msg/String", "ros2msg", "string data"); w.addSchema(sc);
  mcap::Channel a("/chatter", "cdr", sc.id); w.addChannel(a);
  mcap::Channel b("/status", "cdr", sc.id); w.addChannel(b);
  for (int i = 0; i < 1000; i++) {
    std::string d = "message " + std::to_string(i);
    mcap::Message m; m.channelId = (i % 2) ? b.id : a.id; m.sequence = i; m.logTime = 1000000ull * i; m.publishTime = m.logTime;
    m.data = reinterpret_cast<const std::byte *>(d.data()); m.dataSize = d.size();
    if (!w.write(m).ok()) return -2;
  }
  w.close();
  mcap::McapReader r; if (!r.open(path).ok()) return -3;
  int n = 0, bad = 0;
  for (const auto &mv : r.readMessages()) {
    std::string d(reinterpret_cast<const char *>(mv.message.data), mv.message.dataSize);
    if (d != "message " + std::to_string(n)) bad++;
    if (mv.channel->topic != ((n % 2) ? "/status" : "/chatter")) bad++;
    n++;
  }
  r.close(); unlink(path);
  return bad ? -4 : n;
}
int main() {
  QT_BEGIN("mcap");
  int z = roundtrip(mcap::Compression::Zstd, "zstd");
  printf("  info zstd: %d messages read back\n", z);
  QT_CHECK(z == 1000, "zstd chunks: 1000 messages, in order, topics and payloads");
  int l = roundtrip(mcap::Compression::Lz4, "lz4");
  QT_CHECK(l == 1000, "lz4 chunks: 1000 messages, in order, topics and payloads");
  QT_END();
}
