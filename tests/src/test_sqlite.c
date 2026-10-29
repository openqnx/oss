#include "common.h"
#include <sqlite3.h>
int main(void) {
  QT_BEGIN("sqlite");
  sqlite3 *db = NULL;
  QT_CHECK(sqlite3_open(":memory:", &db) == SQLITE_OK, "open in-memory db");
  QT_CHECK(sqlite3_exec(db, "CREATE TABLE ecu(id INTEGER, name TEXT)", 0, 0, 0) == SQLITE_OK, "create table");
  QT_CHECK(sqlite3_exec(db, "INSERT INTO ecu VALUES(1,'gateway'),(2,'adas')", 0, 0, 0) == SQLITE_OK, "insert rows");
  sqlite3_stmt *st = NULL;
  QT_CHECK(sqlite3_prepare_v2(db, "SELECT count(*) FROM ecu", -1, &st, 0) == SQLITE_OK, "prepare select");
  QT_CHECK(sqlite3_step(st) == SQLITE_ROW && sqlite3_column_int(st, 0) == 2, "row count is 2");
  sqlite3_finalize(st);
  sqlite3_close(db);
  printf("  info sqlite %s\n", sqlite3_libversion());
  QT_END();
}
