#include "common.h"
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "riegeli/bytes/fd_reader.h"
#include "riegeli/bytes/fd_writer.h"
#include "riegeli/bytes/string_reader.h"
#include "riegeli/bytes/string_writer.h"
#include "riegeli/digests/crc32c_digester.h"
#include "riegeli/records/record_reader.h"
#include "riegeli/records/record_writer.h"
#include "riegeli/records/records_metadata.pb.h"
/* riegeli: writes 2000 string records into an in-memory riegeli file with
 * each compressor (none, Brotli, Zstd, Snappy) and reads them back, checking
 * every record and that the compressors shrink the data; writes protocol
 * buffer records (RecordsMetadata messages) with the transposed encoding
 * and file metadata, reads both back; writes a real file with FdWriter and
 * seeks back to a remembered record position with FdReader; damages a file
 * in the middle and checks that reading fails without recovery and skips the
 * damaged region with it. Also the CRC-32C digester (QNX's change: abseil's
 * CRC-32C) against the standard check value 0xe3069283 of "123456789". */
static std::string rec(int i) { return "record #" + std::to_string(i) + " " + std::string(i % 50, 'x'); }
enum { N = 2000 };
static bool write_all(std::string *dest, riegeli::RecordWriterBase::Options o) {
  riegeli::RecordWriter<riegeli::StringWriter<std::string *>> w(riegeli::StringWriter<std::string *>(dest), std::move(o));
  for (int i = 0; i < N; i++) if (!w.WriteRecord(rec(i))) return false;
  return w.Close();
}
static int read_all(const std::string &src, bool *all_match) {
  riegeli::RecordReader<riegeli::StringReader<>> r((riegeli::StringReader<>(src)));
  std::string s; int n = 0; *all_match = true;
  while (r.ReadRecord(s)) { if (s != rec(n)) *all_match = false; n++; }
  if (!r.Close()) return -1;
  return n;
}
int main() {
  QT_BEGIN("riegeli");
  riegeli::Crc32cDigester crc; crc.Write("123456789");
  QT_CHECK(crc.Digest() == 0xe3069283u, "CRC-32C(\"123456789\") = 0xe3069283");

  size_t raw = 0; for (int i = 0; i < N; i++) raw += rec(i).size();
  struct { const char *name; riegeli::RecordWriterBase::Options o; } cases[] = {
    { "uncompressed", riegeli::RecordWriterBase::Options().set_uncompressed() },
    { "brotli", riegeli::RecordWriterBase::Options().set_brotli(6) },
    { "zstd", riegeli::RecordWriterBase::Options().set_zstd(3) },
    { "snappy", riegeli::RecordWriterBase::Options().set_snappy() },
  };
  size_t size_uncompressed = 0;
  for (auto &c : cases) {
    std::string file; bool match = false;
    bool w = write_all(&file, c.o);
    int n = w ? read_all(file, &match) : -1;
    printf("  info %-12s %zu bytes (records: %zu bytes)\n", c.name, file.size(), raw);
    char what[96]; snprintf(what, sizeof what, "%s: %d records written and read back unchanged", c.name, N);
    QT_CHECK(w && n == N && match, what);
    if (!strcmp(c.name, "uncompressed")) size_uncompressed = file.size();
    else { snprintf(what, sizeof what, "%s: smaller than uncompressed", c.name);
           QT_CHECK(file.size() * 2 < size_uncompressed, what); }
  }
  { /* proto records, transposed, with file metadata */
    std::string file;
    riegeli::RecordsMetadata meta; meta.set_file_comment("qnx port test");
    {
      riegeli::RecordWriter<riegeli::StringWriter<std::string *>> w(riegeli::StringWriter<std::string *>(&file),
          riegeli::RecordWriterBase::Options().set_transpose(true).set_zstd().set_metadata(meta));
      for (int i = 0; i < 100; i++) { riegeli::RecordsMetadata m; m.set_file_comment(rec(i)); m.set_record_type_name("t" + std::to_string(i % 7)); w.WriteRecord(m); }
      QT_CHECK(w.Close(), "100 proto records written, transposed");
    }
    riegeli::RecordReader<riegeli::StringReader<>> r((riegeli::StringReader<>(file)));
    riegeli::RecordsMetadata got; bool ok = r.ReadMetadata(got);
    QT_CHECK(ok && got.file_comment() == "qnx port test", "file metadata read back");
    int n = 0, match = 1; riegeli::RecordsMetadata m;
    while (r.ReadRecord(m)) { if (m.file_comment() != rec(n) || m.record_type_name() != "t" + std::to_string(n % 7)) match = 0; n++; }
    QT_CHECK(r.Close() && n == 100 && match, "100 proto records parsed back unchanged");
  }
  const char *tmp = getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp";
  char path[256]; snprintf(path, sizeof path, "%s/test_riegeli_%d.riegeli", tmp, (int)getpid());
  { /* a real file, positions and seeking */
    riegeli::RecordPosition pos;
    {
      riegeli::RecordWriter<riegeli::FdWriter<>> w(riegeli::FdWriter<>(path), riegeli::RecordWriterBase::Options().set_zstd().set_chunk_size(4096));
      for (int i = 0; i < N; i++) { w.WriteRecord(rec(i)); if (i == 1234) pos = w.LastPos().get(); }
      QT_CHECK(w.Close(), "file written with FdWriter (small chunks)");
    }
    riegeli::RecordReader<riegeli::FdReader<>> r((riegeli::FdReader<>(path)));
    std::string s;
    QT_CHECK(r.Seek(pos) && r.ReadRecord(s) && s == rec(1234), "seek to the position of record 1234 and read it");
    QT_CHECK(r.ReadRecord(s) && s == rec(1235), "then record 1235 follows");
    QT_CHECK(r.Close(), "reader closed cleanly");
  }
  unlink(path);
  { /* damage and recovery */
    std::string file; write_all(&file, riegeli::RecordWriterBase::Options().set_uncompressed().set_chunk_size(2048));
    for (size_t i = file.size() / 2; i < file.size() / 2 + 64; i++) file[i] ^= 0x5a;
    bool match; int n = read_all(file, &match);
    QT_CHECK(n < 0, "damaged file: reading without recovery fails");
    int skipped = 0, got = 0; std::string s;
    riegeli::RecordReader<riegeli::StringReader<>> r(riegeli::StringReader<>(file),
        riegeli::RecordReaderBase::Options().set_recovery([&](const riegeli::SkippedRegion &, riegeli::RecordReaderBase &) { skipped++; return true; }));
    while (r.ReadRecord(s)) got++;
    printf("  info with recovery: %d of %d records, %d region(s) skipped\n", got, N, skipped);
    QT_CHECK(r.Close() && skipped >= 1 && got > N / 2 && got < N, "with recovery: the damaged region is skipped, the rest read");
  }
  QT_END();
}
