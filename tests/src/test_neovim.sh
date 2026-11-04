#!/bin/sh
# Neovim: headless (nvim --headless) edits a file with ex commands and runs
# Lua through the built-in LuaJIT -- vim.api buffer calls, vim.fn, string and
# table work, a JSON round trip -- results compared with known ones.
# Run the test itself in the background, its output to a log, and wait at
# most 170 s: nothing it leaves behind can hold the runner's output, and a
# hang ends as a FAIL with whatever the test printed so far.
if [ -z "$QNXPORT_INNER" ]; then
  QNXPORT_INNER=/tmp/qnxport-out.$$; export QNXPORT_INNER
  sh "$0" "$@" > "$QNXPORT_INNER" 2>&1 < /dev/null &
  qp=$!; qi=0
  while kill -0 $qp 2>/dev/null && [ $qi -lt 170 ]; do sleep 1; qi=$((qi+1)); done
  if kill -0 $qp 2>/dev/null; then
    for p in $(cat "$QNXPORT_INNER.pids" 2>/dev/null); do kill $p 2>/dev/null; done
    kill $qp 2>/dev/null; sleep 1; kill -KILL $qp 2>/dev/null
    cat "$QNXPORT_INNER"; echo "  FAIL watchdog: still running after 170 s -- stopped"
    echo "FAIL ${0##*/} (watchdog)"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit 1
  fi
  wait $qp; rc=$?; cat "$QNXPORT_INNER"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit $rc
fi
N=neovim
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
NV=$L/bin/nvim
T=$(mktemp -d /tmp/qnxport-$N.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
# Whatever the test starts stops with it, on every exit path (track PID;
# qstop_hook for anything else); the outer shell above kills them on a hang.
QP=$QNXPORT_INNER.pids; : > "$QP"
track() { echo "$1" >> "$QP"; }
qstop_hook() { :; }
stopall() {
  qstop_hook 2>/dev/null
  for p in $(cat "$QP" 2>/dev/null); do kill "$p" 2>/dev/null; done
  : > "$QP"; [ -n "$T" ] && [ -d "$T" ] && { cd /; rm -rf "$T"; }
}
trap stopall EXIT
trap 'exit 143' TERM
HOME=$T; XDG_CONFIG_HOME=$T/cfg; XDG_DATA_HOME=$T/data; XDG_STATE_HOME=$T/state; XDG_CACHE_HOME=$T/cache
export HOME XDG_CONFIG_HOME XDG_DATA_HOME XDG_STATE_HOME XDG_CACHE_HOME
nv() { "$NV" --headless -u NONE -i NONE -n "$@" < /dev/null; }
info "$("$NV" --version 2>&1 | head -n 1)"
printf 'zeta foo\nalpha foo bar\ndelete me\nmid foo\n' > f.txt
nv -c '%s/foo/FOO/g' -c 'g/delete me/d' -c 'sort' -c 'wq' f.txt > log 2>&1
[ "$(cat f.txt | tr '\n' '|')" = "alpha FOO bar|mid FOO|zeta FOO|" ] && ok "headless ex edit: substitute, :g delete, sort, write" || { bad "headless edit"; cat f.txt; head -3 log; }
cat > t.lua <<'EOF'
local buf = vim.api.nvim_get_current_buf()
vim.api.nvim_buf_set_lines(buf, 0, -1, false, {"three", "one", "two"})
table.sort(vim.api.nvim_buf_get_lines(buf, 0, -1, false))
local lines = vim.api.nvim_buf_get_lines(buf, 0, -1, false)
local t = {}
for i = 1, 10 do t[#t + 1] = i * i end
local sum = 0; for _, v in ipairs(t) do sum = sum + v end
local j = vim.json.decode(vim.json.encode({os = "QNX", list = {1, 2, 3}}))
local out = {
  "lines=" .. table.concat(lines, ","),
  "sumsq=" .. sum,
  "upper=" .. string.upper("neovim"),
  "fn=" .. vim.fn.toupper("x") .. vim.fn.strlen("qnx"),
  "json=" .. j.os .. #j.list,
  "jit=" .. tostring(jit ~= nil),
  "split=" .. table.concat(vim.split("a,b,c", ","), "+"),
}
vim.fn.writefile(out, "out.txt")
vim.cmd("qall!")
EOF
nv -c 'luafile t.lua' > log2 2>&1
exp='lines=three,one,two
sumsq=385
upper=NEOVIM
fn=X3
json=QNX3
jit=true
split=a+b+c'
[ "$(cat out.txt 2>/dev/null)" = "$exp" ] && ok "Lua: nvim_buf_* API, vim.fn, vim.json, vim.split, LuaJIT" || { bad "Lua script"; cat out.txt 2>/dev/null; head -5 log2; }
r=$(nv -c 'lua io.stdout:write(vim.version().major .. "." .. vim.version().minor .. "\n")' -c 'qall!' 2>&1)
[ -n "$r" ] && ok "vim.version(): $r" || bad "vim.version()"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
