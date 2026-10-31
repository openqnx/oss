// sqlite_orm (header-only ORM over SQLite): a typed schema synced into a
// database file in /tmp, inserts with autoincrement ids, queries with where /
// order_by / aggregates / a join, update, remove, transactions rolled back
// and committed, and the database file reopened by a second storage.
#include "common.h"
#include <sqlite_orm/sqlite_orm.h>
#include <cstdio>
#include <string>
#include <cstdlib>
#include <unistd.h>

using namespace sqlite_orm;
struct User { int id; std::string name; int age; };
struct Pet { int id; int owner; std::string kind; };

static auto make(const std::string &path) {
  return make_storage(path,
    make_table("users", make_column("id", &User::id, primary_key().autoincrement()),
               make_column("name", &User::name), make_column("age", &User::age)),
    make_table("pets", make_column("id", &Pet::id, primary_key().autoincrement()),
               make_column("owner", &Pet::owner), make_column("kind", &Pet::kind)));
}

int main() {
  QT_BEGIN("sqlite_orm");
  const char *tmp = getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp";
  char path[256]; snprintf(path, sizeof path, "%s/qnxport-sqlite-orm-%d.db", tmp, (int)getpid());
  unlink(path);
  try {
    auto st = make(path);
    st.sync_schema();
    QT_CHECK(st.table_exists("users") && st.table_exists("pets"), "sync_schema created both tables");
    int ida = st.insert(User{0, "alice", 34}), idb = st.insert(User{0, "bob", 27}), idc = st.insert(User{0, "carol", 41});
    QT_CHECK(ida == 1 && idb == 2 && idc == 3, "insert returns autoincrement ids 1,2,3");
    st.insert(Pet{0, ida, "cat"}); st.insert(Pet{0, ida, "dog"}); st.insert(Pet{0, idc, "fish"});
    QT_CHECK(st.count<User>() == 3 && st.count<Pet>() == 3, "count");
    auto older = st.get_all<User>(where(c(&User::age) > 30), order_by(&User::name));
    QT_CHECK(older.size() == 2 && older[0].name == "alice" && older[1].name == "carol", "where age > 30 order by name");
    auto total = st.sum(&User::age);
    QT_CHECK(total && *total == 102, "sum(age) = 102");
    auto mx = st.max(&User::age);
    QT_CHECK(mx && *mx == 41, "max(age) = 41");
    auto rows = st.select(columns(&User::name, count(&Pet::id)), inner_join<Pet>(on(c(&Pet::owner) == &User::id)),
                          group_by(&User::name), order_by(&User::name));
    QT_CHECK(rows.size() == 2 && std::get<0>(rows[0]) == "alice" && std::get<1>(rows[0]) == 2, "join + group by: alice has 2 pets");
    User bob = st.get<User>(idb);
    bob.age = 28; st.update(bob);
    QT_CHECK(st.get<User>(idb).age == 28, "update");
    st.remove<User>(idc);
    QT_CHECK(st.count<User>() == 2 && !st.get_pointer<User>(idc), "remove");
    st.transaction([&] { st.insert(User{0, "dave", 50}); return false; });   // rolled back
    QT_CHECK(st.count<User>() == 2, "transaction rolled back");
    st.transaction([&] { st.insert(User{0, "erin", 22}); return true; });
    QT_CHECK(st.count<User>() == 3, "transaction committed");
    bool threw = false;
    try { st.get<User>(999); } catch (const std::system_error &) { threw = true; }
    QT_CHECK(threw, "get of a missing id throws");
  } catch (std::exception &e) {
    printf("  info exception: %s\n", e.what());
    QT_CHECK(false, "no exception");
  }
  try {  // a second storage on the same file sees the committed data
    auto st2 = make(path);
    QT_CHECK(st2.count<User>() == 3 && st2.count<Pet>() == 3, "data persisted in the database file");
  } catch (std::exception &e) {
    printf("  info exception: %s\n", e.what());
    QT_CHECK(false, "reopen the database file");
  }
  unlink(path);
  QT_END();
}
