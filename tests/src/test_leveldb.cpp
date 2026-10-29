// LevelDB (+snappy): put/get/iterate on a database in /tmp.
#include "common.h"
#include <leveldb/db.h>
#include <leveldb/options.h>
#include <cstdlib>
int main() {
  QT_BEGIN("leveldb");
  std::string dir = "/tmp/qnx-leveldb-test"; system(("rm -rf " + dir).c_str());
  leveldb::DB *db; leveldb::Options o; o.create_if_missing = true; o.compression = leveldb::kSnappyCompression;
  QT_CHECK(leveldb::DB::Open(o, dir, &db).ok(), "open");
  for (int i = 0; i < 100; i++) db->Put(leveldb::WriteOptions(), "k" + std::to_string(i), std::string(100, 'v'));
  std::string v; QT_CHECK(db->Get(leveldb::ReadOptions(), "k42", &v).ok() && v.size() == 100, "get");
  int n = 0; auto *it = db->NewIterator(leveldb::ReadOptions()); for (it->SeekToFirst(); it->Valid(); it->Next()) n++;
  QT_CHECK(n == 100, "iterate");
  delete it; delete db; system(("rm -rf " + dir).c_str());
  QT_END();
}
