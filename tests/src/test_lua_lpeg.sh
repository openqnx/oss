#!/bin/sh
# lua-lpeg: LPeg as LuaJIT loads it (lib/lua/5.1/lpeg.so, plus the re module
# share/lua/5.1/re.lua), driven by the ported LuaJIT interpreter. Known
# answers: an arithmetic grammar evaluated through captures, CSV fields,
# substitutions, re.match/re.find/re.gsub, and patterns that must not match.
echo "== lua-lpeg =="
P=${QNX_PORT_PREFIX:-/usr/local}
LJ=$P/bin/luajit
[ -x "$LJ" ] || LJ=$(command -v luajit 2>/dev/null)
if [ -z "$LJ" ] || [ ! -x "$LJ" ]; then
  echo "  FAIL luajit interpreter found ($P/bin/luajit)"; echo "FAIL lua-lpeg"; exit 1
fi
echo "  ok   luajit interpreter found ($LJ)"
LUA_CPATH="$P/lib/lua/5.1/?.so;;"; LUA_PATH="$P/share/lua/5.1/?.lua;;"; export LUA_CPATH LUA_PATH
out=$("$LJ" - 2>&1 <<'LUAEOF'
local fails = 0
local function check(c, what) if c then print("  ok   " .. what) else print("  FAIL " .. what); fails = fails + 1 end end
local okl, lpeg = pcall(require, "lpeg")
check(okl and type(lpeg) == "table", "require 'lpeg' (lpeg.so)")
if not okl then print("  info " .. tostring(lpeg)); os.exit(1) end
print("  info " .. tostring(lpeg.version and (type(lpeg.version) == "function" and lpeg.version() or lpeg.version)) .. " on " .. jit.version)
local P, R, S, C, Ct, Cs, Cf, Cg, V = lpeg.P, lpeg.R, lpeg.S, lpeg.C, lpeg.Ct, lpeg.Cs, lpeg.Cf, lpeg.Cg, lpeg.V
check(lpeg.match(P"hello", "hello world") == 6, "literal match returns the end position (6)")
check(lpeg.match(P"world", "hello world") == nil, "anchored literal does not match elsewhere")
-- arithmetic with precedence and parentheses, evaluated through folding captures
local space = S" \t"^0
local num = C(P"-"^-1 * R"09"^1) / tonumber * space
local addop, mulop = C(S"+-") * space, C(S"*/") * space
local function ev(a, op, b)
  if op == "+" then return a + b elseif op == "-" then return a - b elseif op == "*" then return a * b else return a / b end
end
local G = P{ "Exp",
  Exp = Cf(V"Term" * Cg(addop * V"Term")^0, ev),
  Term = Cf(V"Fac" * Cg(mulop * V"Fac")^0, ev),
  Fac = num + P"(" * space * V"Exp" * P")" * space }
local calc = space * G * -1
check(lpeg.match(calc, "3 + 4 * (2 - 1)") == 7, "grammar: 3 + 4 * (2 - 1) = 7")
check(lpeg.match(calc, "2*(3+4)*5 - -6/3") == 72, "grammar: 2*(3+4)*5 - -6/3 = 72")
check(lpeg.match(calc, "2 + * 3") == nil, "grammar rejects '2 + * 3'")
-- CSV fields with quoted commas
local field = '"' * Cs(((P(1) - '"') + P'""' / '"')^0) * '"' + C((1 - S',\n"')^0)
local record = Ct(field * (',' * field)^0) * (P"\n" + -1)
local t = lpeg.match(record, 'a,"b,c",,"say ""hi"""')
check(t and #t == 4 and t[1] == "a" and t[2] == "b,c" and t[3] == "" and t[4] == 'say "hi"', "CSV record: 4 fields incl. quoted comma and escaped quote")
-- substitution
local vowel = S"aeiou" / string.upper
check(lpeg.match(Cs((vowel + 1)^0), "quick brown fox") == "qUIck brOwn fOx", "Cs substitution uppercases vowels")
-- position and table captures
local words = Ct((C(R"az"^1) + 1)^0)
local w = lpeg.match(words, "one, two;three")
check(w and table.concat(w, "|") == "one|two|three", "Ct collects words one|two|three")
-- re module
local okr, re = pcall(require, "re")
check(okr and type(re) == "table", "require 're' (re.lua)")
if okr then
  check(re.match("the number 423 is odd", "({%a+} / .)*") == "the", "re.match returns the first capture")
  check(re.find("the number 423 is odd", "[0-9]+") == 12, "re.find locates 423 at 12")
  check(re.gsub("hello world", "'o'", "0") == "hell0 w0rld", "re.gsub replaces")
  local date = re.compile("{:y: %d^4 :} '-' {:m: %d^2 :} '-' {:d: %d^2 :}", nil)
  local dt = lpeg.match(Ct(date), "2026-10-31")
  check(dt and dt.y == "2026" and dt.m == "10" and dt.d == "31", "re named group captures (2026-10-31)")
  local list = re.compile[[ list <- {| item (',' item)* |} !.   item <- %s* { [a-z]+ } %s* ]]
  local l = list:match(" alpha , beta,gamma ")
  check(l and #l == 3 and l[3] == "gamma", "re grammar parses a list of 3 names")
  check(list:match("alpha,,beta") == nil, "re grammar rejects an empty item")
end
print("  ok   lua script ran to completion")
os.exit(fails == 0 and 0 or 1)
LUAEOF
)
rc=$?
echo "$out"
if [ $rc -eq 0 ] && echo "$out" | grep -q "ran to completion" && ! echo "$out" | grep -q "^  FAIL"; then
  echo "PASS lua-lpeg"; exit 0
fi
echo "FAIL lua-lpeg (luajit rc=$rc)"; exit 1
