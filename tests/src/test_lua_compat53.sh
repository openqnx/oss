#!/bin/sh
# lua-compat53: Lua 5.3 APIs on the older interpreters. Each of lua5.1,
# lua5.2 and luajit (5.1 modules) runs the same script: require "compat53"
# and the C modules it loads (compat53.string, .table, .utf8 from
# lib/lua/5.x/compat53) give string.pack/unpack, utf8, table.move,
# math.tointeger/math.type, table.unpack, xpcall with arguments, load with a
# mode and an environment, and string.rep with a separator -- known answers.
# compat53.module provides the same without changing the globals. On lua5.3
# require "compat53" must load and change nothing.
echo "== lua-compat53 =="
P=${QNX_PORT_PREFIX:-/usr/local}
fails=0
SCRIPT='
local fails = 0
local function check(c, what) if c then print("  ok   " .. what) else print("  FAIL " .. what); fails = fails + 1 end end
local V = (jit and jit.version) or _VERSION
local native_move = table.move
local okm, M = pcall(require, "compat53.module")
check(okm and type(M) == "table" and type(M.table.move) == "function", V .. ": require compat53.module")
check(table.move == native_move, V .. ": compat53.module leaves the globals alone")
local ok, e = pcall(require, "compat53")
check(ok, V .. ": require compat53" .. (ok and "" or (" (" .. tostring(e) .. ")")))
local s = string.pack("<i4 z", 258, "ab")
local n, z = string.unpack("<i4 z", s)
check(#s == 7 and n == 258 and z == "ab" and s:byte(1) == 2 and s:byte(2) == 1, V .. ": string.pack/unpack (C module compat53.string)")
check(utf8 and utf8.char(72, 228, 8364) == "H\195\164\226\130\172" and utf8.len("h\195\164\226\130\172") == 3, V .. ": utf8.char / utf8.len (compat53.utf8)")
local cps = {} for _, c in utf8.codes("a\226\130\172") do cps[#cps + 1] = c end
check(cps[1] == 97 and cps[2] == 8364 and #cps == 2, V .. ": utf8.codes")
local t = table.move({1, 2, 3}, 1, 3, 2, {9})
check(table.concat(t, ",") == "9,1,2,3", V .. ": table.move")
check(math.tointeger(3.0) == 3 and math.tointeger(3.5) == nil and math.type(1) ~= nil and math.type("x") == nil, V .. ": math.tointeger / math.type")
check(select("#", table.unpack({1, 2, 3})) == 3, V .. ": table.unpack")
local okx, r = xpcall(function(a, b) return a + b end, debug.traceback, 2, 3)
check(okx and r == 5, V .. ": xpcall passes arguments")
local f = load("return x * 2", "chunk", "t", {x = 21})
check(f and f() == 42, V .. ": load with mode and environment")
check(string.rep("ab", 3, "-") == "ab-ab-ab", V .. ": string.rep with a separator")
check(math.maxinteger ~= nil and math.mininteger ~= nil and math.ult(1, -1), V .. ": math.maxinteger / math.ult")
print("  ok   " .. V .. ": script ran to completion")
os.exit(fails == 0 and 0 or 1)
'
for L in lua5.1 lua5.2 luajit; do
  B=$P/bin/$L
  if [ ! -x "$B" ]; then echo "  FAIL $L installed ($B)"; fails=$((fails + 1)); continue; fi
  out=$("$B" -e "$SCRIPT" 2>&1); rc=$?
  echo "$out"
  if [ $rc -ne 0 ] || ! echo "$out" | grep -q "script ran to completion"; then fails=$((fails + 1)); echo "  FAIL $L (rc $rc)"; fi
done
if [ -x "$P/bin/lua5.3" ]; then
  out=$("$P/bin/lua5.3" -e 'local m = table.move; local u = utf8; require "compat53"; print(table.move == m and utf8 == u and math.type(1) == "integer")' 2>&1)
  [ "$out" = "true" ] && echo "  ok   lua5.3: require compat53 loads and changes nothing" || { echo "  FAIL lua5.3: require compat53 ($out)"; fails=$((fails + 1)); }
fi
[ $fails -eq 0 ] && { echo "PASS lua-compat53"; exit 0; }
echo "FAIL lua-compat53 ($fails)"; exit 1
