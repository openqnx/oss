#!/bin/sh
# luarocks: the package manager on the target, offline. Known answers:
# luarocks --version and its configuration (Lua 5.3, the interpreter, the
# system tree); luarocks list on the system tree; a pure-Lua rock built from
# a local rockspec whose source is a local .tar.gz (file:// URL: copied,
# unpacked with tar and gzip, installed by the builtin build type) into a
# tree of its own, listed, loaded by lua5.3 through `luarocks path`, and
# removed again; the same rock for Lua 5.2 (--lua-version=5.2) run by lua5.2.
echo "== luarocks =="
P=${QNX_PORT_PREFIX:-/usr/local}
LR=$P/bin/luarocks
fails=0
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fails=$((fails + 1)); }
chk() { if [ "$2" = "$3" ]; then ok "$1"; else bad "$1"; echo "  info want: $3"; echo "  info got:  $2"; fi; }
[ -x "$LR" ] || { echo "  FAIL luarocks installed ($LR)"; echo "FAIL luarocks"; exit 1; }
# a clean environment: no user configuration, no proxies to go to
T=${TMPDIR:-/tmp}/luarocks-test.$$; rm -rf "$T"; mkdir -p "$T/home"
HOME=$T/home; export HOME; unset LUA_PATH LUA_CPATH LUA_PATH_5_2 LUA_PATH_5_3 LUA_CPATH_5_2 LUA_CPATH_5_3

chk "luarocks --version" "$("$LR" --version 2>&1 | head -n 1)" "$LR 3.12.2"
chk "luarocks config lua_version" "$("$LR" config lua_version 2>&1)" "5.3"
chk "luarocks config variables.LUA" "$("$LR" config variables.LUA 2>&1)" "/usr/local/bin/lua5.3"
out=$("$LR" list 2>&1); rc=$?
if [ $rc -eq 0 ] && echo "$out" | grep -q "Rocks installed for Lua 5.3"; then ok "luarocks list (system tree, rc 0)"; else bad "luarocks list (rc $rc)"; echo "$out" | head -5; fi

# the rock: two modules, one using the other
mkdir -p "$T/w/qnxhello-1.0/src/qnxhello"
cat > "$T/w/qnxhello-1.0/src/qnxhello/init.lua" <<'L'
local util = require "qnxhello.util"
local M = {}
function M.greet(who) return "hello, " .. util.upper(who) .. " (" .. _VERSION .. ")" end
return M
L
cat > "$T/w/qnxhello-1.0/src/qnxhello/util.lua" <<'L'
return { upper = function(s) return (s:gsub("%a", string.upper)) end }
L
( cd "$T/w" && tar cf - qnxhello-1.0 | gzip -c > "$T/qnxhello-1.0.tar.gz" )
cat > "$T/qnxhello-1.0-1.rockspec" <<R
package = "qnxhello"
version = "1.0-1"
source = { url = "file://$T/qnxhello-1.0.tar.gz", dir = "qnxhello-1.0" }
description = { summary = "test rock", license = "MIT" }
dependencies = { "lua >= 5.1" }
build = { type = "builtin", modules = {
  ["qnxhello"] = "src/qnxhello/init.lua",
  ["qnxhello.util"] = "src/qnxhello/util.lua" } }
R
cd "$T" || exit 1
out=$("$LR" install --tree="$T/tree" "$T/qnxhello-1.0-1.rockspec" 2>&1); rc=$?
if [ $rc -eq 0 ] && [ -f "$T/tree/share/lua/5.3/qnxhello/init.lua" ] && [ -f "$T/tree/share/lua/5.3/qnxhello/util.lua" ]; then
  ok "luarocks install from a local rockspec + .tar.gz: modules in share/lua/5.3"
else bad "luarocks install from a local rockspec (rc $rc)"; echo "$out" | tail -8; fi
out=$("$LR" list --tree="$T/tree" --porcelain 2>&1)
chk "luarocks list --porcelain shows it" "$out" "qnxhello	1.0-1	installed	$T/tree/lib/luarocks/rocks-5.3"
out=$("$LR" show --tree="$T/tree" qnxhello 2>&1 | grep -c "qnxhello")
[ "$out" -ge 3 ] && ok "luarocks show qnxhello" || bad "luarocks show qnxhello"
eval "$("$LR" path --tree="$T/tree" 2>/dev/null)"
out=$("$P/bin/lua5.3" -e 'print(require("qnxhello").greet("qnx"))' 2>&1)
chk "lua5.3 loads the rock through luarocks path" "$out" "hello, QNX (Lua 5.3)"
unset LUA_PATH LUA_CPATH LUA_PATH_5_3 LUA_CPATH_5_3
out=$("$LR" remove --tree="$T/tree" qnxhello 2>&1); rc=$?
if [ $rc -eq 0 ] && [ ! -e "$T/tree/share/lua/5.3/qnxhello/init.lua" ] && [ -z "$("$LR" list --tree="$T/tree" --porcelain 2>&1)" ]; then
  ok "luarocks remove: files and manifest entry gone"
else bad "luarocks remove (rc $rc)"; echo "$out" | tail -5; fi

if [ -x "$P/bin/lua5.2" ]; then
  out=$("$LR" --lua-version=5.2 install --tree="$T/tree52" "$T/qnxhello-1.0-1.rockspec" 2>&1); rc=$?
  [ $rc -eq 0 ] && [ -f "$T/tree52/share/lua/5.2/qnxhello/init.lua" ] && ok "--lua-version=5.2 install: share/lua/5.2" \
    || { bad "--lua-version=5.2 install (rc $rc)"; echo "$out" | tail -5; }
  out=$(LUA_PATH="$T/tree52/share/lua/5.2/?.lua;$T/tree52/share/lua/5.2/?/init.lua;;" "$P/bin/lua5.2" -e 'print(require("qnxhello").greet("qnx"))' 2>&1)
  chk "lua5.2 loads it" "$out" "hello, QNX (Lua 5.2)"
else
  echo "  info lua5.2 not installed: --lua-version=5.2 skipped"
fi
cd / && rm -rf "$T"
[ $fails -eq 0 ] && { echo "PASS luarocks"; exit 0; }
echo "FAIL luarocks ($fails)"; exit 1
