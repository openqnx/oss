# LuaRocks, the Lua package manager: luarocks and luarocks-admin, run by the
# Lua 5.3 interpreter (lua5.3: the newest Lua here with an interpreter), its
# modules in share/lua/5.3/luarocks. It installs rocks for Lua 5.3 by
# default and, with --lua-version=5.1 / 5.2, for the lua5.1 (and LuaJIT) and
# lua5.2 interpreters: one configuration per version in etc/luarocks. The
# rocks tree is the prefix (share/lua/5.x, lib/lua/5.x, lib/luarocks/
# rocks-5.x), where the interpreters look by default. Fetching from the
# network uses curl; .tar.gz sources are unpacked with tar and gzip (all in
# the QNX base system); .rock/.zip archives need zip/unzip. Version as in
# QNX's own package list.
VERSION=3.12.2
TARBALL=https://luarocks.org/releases/luarocks-3.12.2.tar.gz
SHA256=b0e0c85205841ddd7be485f53d6125766d18a81d226588d2366931e9a1484492
BUILDSYS=custom
DEPS=""
RUNTIME_DEPS="lua5.3"
# upstream's configure runs the Lua interpreter to find out about it, which
# a cross build cannot do: write what it would have found (config.unix) and
# let the makefile generate the scripts and configuration from it.
do_configure() {
  cat > "$S/config.unix" <<CFG
prefix=$PREFIX
sysconfdir=$PREFIX/etc
rocks_tree=$PREFIX
LUA_VERSION=5.3
LUA=$PREFIX/bin/lua5.3
LUA_DIR=$PREFIX
LUA_BINDIR=$PREFIX/bin
LUA_INCDIR=$PREFIX/include/lua5.3
LUA_LIBDIR=$PREFIX/lib
FORCE_CONFIG=
CFG
}
do_build() { make -C "$S" build; }
# The configuration of one Lua version: the interpreter and its headers and
# library (C rocks), as LuaRocks' configure writes it. LUALIB names the
# versioned library (liblua5.<n>.so): LuaRocks would look for liblua.so.
lr_config() {
  local v=$1 lua=$2
  cat <<CFG
-- LuaRocks configuration for Lua $v
rocks_trees = {
   { name = "user", root = home .. "/.luarocks" };
   { name = "system", root = "$PREFIX" };
}
variables = {
   LUA_DIR = "$PREFIX";
   LUA_INCDIR = "$PREFIX/include/lua$v";
   LUA_BINDIR = "$PREFIX/bin";
   LUA_LIBDIR = "$PREFIX/lib";
   LUA_VERSION = "$v";
   LUA = "$PREFIX/bin/$lua";
   LUALIB = "liblua$v.so";
}
CFG
}
do_install() {
  make -C "$S" install DESTDIR="$STAGE" || return 1
  lr_config 5.3 lua5.3 > "$STAGE$PREFIX/etc/luarocks/config-5.3.lua"
  lr_config 5.2 lua5.2 > "$STAGE$PREFIX/etc/luarocks/config-5.2.lua"
  lr_config 5.1 lua5.1 > "$STAGE$PREFIX/etc/luarocks/config-5.1.lua"
}
