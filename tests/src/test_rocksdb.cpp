// RocksDB in a directory in /tmp: open with create_if_missing (LZ4 for the
// upper levels, ZSTD at the bottom), Put/Get/Delete, an atomic WriteBatch,
// ordered iteration and Seek, a snapshot that keeps seeing old values, a
// column family, a custom merge operator (64-bit counter), Flush and a full
// CompactRange that leaves the keys in SST files on one level with
// tombstones dropped, reopening the database (data persisted, merge
// operands applied), an optimistic transaction that conflicts, a checkpoint
// opened read-only, the ldb and sst_dump tools reading the files, and
// DestroyDB removing it all.
#include "common.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/write_batch.h>
#include <rocksdb/merge_operator.h>
#include <rocksdb/utilities/checkpoint.h>
#include <rocksdb/utilities/optimistic_transaction_db.h>
#include <rocksdb/utilities/transaction.h>
#include <rocksdb/convenience.h>

using namespace ROCKSDB_NAMESPACE;

class Add64 : public AssociativeMergeOperator {
 public:
  bool Merge(const Slice &, const Slice *existing, const Slice &value, std::string *out, Logger *) const override {
    uint64_t a = existing ? std::stoull(existing->ToString()) : 0;
    *out = std::to_string(a + std::stoull(value.ToString()));
    return true;
  }
  const char *Name() const override { return "Add64"; }
};
static std::string key(int i) { char b[16]; snprintf(b, sizeof b, "key%05d", i); return b; }
static int sh(const std::string &cmd, std::string &out) {
  out.clear(); FILE *p = popen(cmd.c_str(), "r"); if (!p) return -1;
  char b[4096]; size_t n;
  while ((n = fread(b, 1, sizeof b, p)) > 0) out.append(b, n);
  int st = pclose(p);
  return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}

int main() {
  QT_BEGIN("rocksdb");
  std::string dir = "/tmp/qnxport-rocksdb-" + std::to_string(getpid());
  std::string path = dir + "/db", cp = dir + "/checkpoint";
  mkdir(dir.c_str(), 0755);
  Options opt;
  opt.create_if_missing = true;
  opt.compression = kLZ4Compression;
  opt.bottommost_compression = kZSTD;
  opt.merge_operator = std::make_shared<Add64>();
  opt.write_buffer_size = 256 * 1024;
  opt.max_background_jobs = 2;
  // RocksDB 11 hands the database out as a std::unique_ptr
  std::unique_ptr<DB> db;
  Status s = DB::Open(opt, path, &db);
  QT_CHECK(s.ok(), "DB::Open with create_if_missing");
  if (!s.ok()) { printf("  info %s\n", s.ToString().c_str()); QT_END(); }
  WriteOptions wo; ReadOptions ro; std::string v;

  QT_CHECK(db->Put(wo, "hello", "world").ok() && db->Get(ro, "hello", &v).ok() && v == "world", "Put/Get");
  QT_CHECK(db->Delete(wo, "hello").ok() && db->Get(ro, "hello", &v).IsNotFound(), "Delete -> NotFound");
  WriteBatch wb;
  for (int i = 0; i < 5000; i++) wb.Put(key(i), std::string(100, char('a' + i % 26)));
  wb.Delete(key(13));
  QT_CHECK(db->Write(wo, &wb).ok(), "WriteBatch of 5000 puts and a delete");
  QT_CHECK(db->Get(ro, key(4999), &v).ok() && v.size() == 100 && v[0] == char('a' + 4999 % 26) && db->Get(ro, key(13), &v).IsNotFound(),
           "batch applied atomically");

  const Snapshot *snap = db->GetSnapshot();
  db->Put(wo, key(1), "changed");
  ReadOptions sro; sro.snapshot = snap;
  QT_CHECK(db->Get(sro, key(1), &v).ok() && v == std::string(100, 'b') && db->Get(ro, key(1), &v).ok() && v == "changed",
           "snapshot keeps the old value");
  db->ReleaseSnapshot(snap);

  {
    std::unique_ptr<Iterator> it(db->NewIterator(ro));
    int n = 0; std::string prev; bool ordered = true;
    for (it->SeekToFirst(); it->Valid(); it->Next()) { if (n && it->key().ToString() <= prev) ordered = false; prev = it->key().ToString(); n++; }
    QT_CHECK(it->status().ok() && n == 4999 && ordered, "iteration: 4999 keys in order");
    it->Seek("key02500x");
    QT_CHECK(it->Valid() && it->key() == key(2501), "Seek lands on the next key");
    it->SeekForPrev("key00013");
    QT_CHECK(it->Valid() && it->key() == key(12), "SeekForPrev skips the deleted key");
  }

  ColumnFamilyHandle *cf = nullptr;
  QT_CHECK(db->CreateColumnFamily(ColumnFamilyOptions(opt), "meta", &cf).ok(), "CreateColumnFamily meta");
  db->Put(wo, cf, "version", "1");
  QT_CHECK(db->Get(ro, cf, "version", &v).ok() && v == "1" && db->Get(ro, "version", &v).IsNotFound(), "column families are separate");

  for (int i = 0; i < 10; i++) db->Merge(wo, "counter", std::to_string(i + 1));
  QT_CHECK(db->Get(ro, "counter", &v).ok() && v == "55", "merge operator: 1+2+...+10 = 55");

  QT_CHECK(db->Flush(FlushOptions()).ok(), "Flush");
  for (int i = 0; i < 1000; i++) db->Delete(wo, key(i));
  CompactRangeOptions cro; cro.bottommost_level_compaction = BottommostLevelCompaction::kForce;
  s = db->CompactRange(cro, nullptr, nullptr);
  QT_CHECK(s.ok(), "CompactRange over the whole key space");
  std::string files, est;
  db->GetProperty("rocksdb.num-files-at-level0", &files);
  db->GetProperty("rocksdb.estimate-num-keys", &est);
  printf("  info after compaction: %s file(s) at L0, estimate-num-keys %s\n", files.c_str(), est.c_str());
  std::vector<LiveFileMetaData> meta; db->GetLiveFilesMetaData(&meta);
  int levels_used = 0, last = -1; uint64_t entries = 0;
  for (auto &m : meta) { if (m.column_family_name == "default") { if (m.level != last) { levels_used++; last = m.level; } entries += m.num_entries; } }
  printf("  info %zu live SST file(s), %llu entries in default\n", meta.size(), (unsigned long long)entries);
  QT_CHECK(files == "0" && levels_used == 1 && entries == 4001, "compacted to one level: 4000 keys + the merged counter, tombstones dropped");
  {
    std::unique_ptr<Iterator> it(db->NewIterator(ro));
    it->Seek("key");      // ("counter" sorts before the key%05d keys)
    QT_CHECK(it->Valid() && it->key() == key(1000), "first key after the deletions is key01000");
  }

  {
    Checkpoint *ck = nullptr;
    QT_CHECK(Checkpoint::Create(db.get(), &ck).ok() && ck->CreateCheckpoint(cp).ok(), "checkpoint created");
    delete ck;
  }
  db->DestroyColumnFamilyHandle(cf);
  QT_CHECK(db->Close().ok(), "Close");
  db.reset();

  // reopen: data persisted
  std::vector<ColumnFamilyDescriptor> cfs = { { kDefaultColumnFamilyName, ColumnFamilyOptions(opt) }, { "meta", ColumnFamilyOptions(opt) } };
  std::vector<ColumnFamilyHandle *> hs;
  s = DB::Open(opt, path, cfs, &hs, &db);
  QT_CHECK(s.ok(), "reopen with both column families");
  if (s.ok()) {
    QT_CHECK(db->Get(ro, key(4321), &v).ok() && v.size() == 100 && db->Get(ro, key(999), &v).IsNotFound() &&
             db->Get(ro, "counter", &v).ok() && v == "55" && db->Get(ro, hs[1], "version", &v).ok() && v == "1",
             "data, deletions, merges and the column family survived");
    for (auto h : hs) db->DestroyColumnFamilyHandle(h);
    db->Close(); db.reset();
  }

  // read-only checkpoint
  {
    std::unique_ptr<DB> ckdb;
    std::vector<ColumnFamilyHandle *> ch;
    s = DB::OpenForReadOnly(opt, cp, cfs, &ch, &ckdb);
    QT_CHECK(s.ok() && ckdb->Get(ro, key(2000), &v).ok() && ckdb->Put(wo, "x", "y").IsNotSupported(), "checkpoint opens read-only with the data");
    if (ckdb) { for (auto h : ch) ckdb->DestroyColumnFamilyHandle(h); ckdb.reset(); }
  }

  // optimistic transactions
  {
    OptimisticTransactionDB *tdb = nullptr;
    std::vector<ColumnFamilyHandle *> th;
    s = OptimisticTransactionDB::Open(opt, path, cfs, &th, &tdb);
    QT_CHECK(s.ok(), "OptimisticTransactionDB::Open");
    if (s.ok()) {
      Transaction *t1 = tdb->BeginTransaction(wo);
      std::string got;
      t1->GetForUpdate(ro, "acct", &got);
      t1->Put("acct", "t1");
      tdb->Put(wo, "acct", "outside");          // conflicting write after t1 read it
      Status c = t1->Commit();
      QT_CHECK(c.IsBusy(), "conflicting optimistic transaction fails to commit (Busy)");
      delete t1;
      Transaction *t2 = tdb->BeginTransaction(wo);
      t2->Put("acct", "t2"); t2->Put("acct2", "t2");
      QT_CHECK(t2->Commit().ok() && tdb->Get(ro, "acct2", &got).ok() && got == "t2", "a clean transaction commits both keys");
      delete t2;
      for (auto h : th) tdb->DestroyColumnFamilyHandle(h);
      delete tdb;
    }
  }

  // tools
  std::string out;
  int rc = sh("{ /usr/local/bin/ldb --db=" + path + " get key04321 || ldb --db=" + path + " get key04321; } 2>&1", out);
  QT_CHECK(rc == 0 && out.size() >= 100 && out.compare(0, 5, std::string(5, char('a' + 4321 % 26))) == 0, "ldb get reads a value");
  rc = sh("{ /usr/local/bin/ldb --db=" + path + " list_column_families || ldb --db=" + path + " list_column_families; } 2>&1", out);
  QT_CHECK(rc == 0 && out.find("meta") != std::string::npos, "ldb list_column_families names meta");
  std::string sst;
  for (auto &m : meta) if (m.column_family_name == "default" && m.num_entries > 0) { sst = path + m.name; break; }
  rc = sh("{ /usr/local/bin/sst_dump --file=" + sst + " --command=check --show_properties || sst_dump --file=" + sst + " --command=check --show_properties; } 2>&1", out);
  printf("  info sst_dump %s: rc %d\n", sst.c_str(), rc);
  { std::string o = out; size_t p; while ((p = o.find('\n')) != std::string::npos && o.size()) { if (o.compare(0, p, "") && (o.find("ompression") < p || rc)) printf("  info   %s\n", o.substr(0, p).c_str()); o.erase(0, p + 1); } }
  // RocksDB 11 names the codec by number ("SST file compression algo:
  // BuiltinV2;07;", options "_type=7"): kZSTD is 0x07, the bottommost level's
  QT_CHECK(rc == 0 && out.find("SST file compression algo:") != std::string::npos &&
           (out.find("_type=7;") != std::string::npos || out.find("ZSTD") != std::string::npos),
           "sst_dump checks the bottommost SST file: compressed with ZSTD");

  QT_CHECK(DestroyDB(path, opt).ok(), "DestroyDB");
  struct stat st;
  QT_CHECK(stat((path + "/CURRENT").c_str(), &st) != 0, "the database files are gone");
  (void)!system(("rm -rf " + dir).c_str());
  QT_END();
}
