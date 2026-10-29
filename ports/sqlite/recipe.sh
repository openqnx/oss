VERSION=3.47.2
# Also linked at run time (found by scripts/gen-dependency-index): zlib
DEPS="zlib"
TARBALL=https://www.sqlite.org/2024/sqlite-autoconf-3470200.tar.gz
SHA256=f1b2ee412c28d7472bc95ba996368d6f0cdcf00362affdadb27ed286c179540b
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --enable-static --disable-readline
                  --disable-editline --enable-threadsafe --enable-session )
# The session extension (with the pre-update hook) and column metadata, as
# Node.js builds its bundled SQLite: node:sqlite calls sqlite3session_*,
# sqlite3changeset_apply and sqlite3_column_origin_name.
CFLAGS_APPEND="-DSQLITE_ENABLE_COLUMN_METADATA"
