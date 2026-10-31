#!/bin/sh
# sysbench, short runs: cpu (primes), memory, threads, mutex, fileio
# prepare/run/cleanup in the temp dir, and a custom Lua test script run by
# sysbench's LuaJIT (prepare hook output and an exact event count).
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
N=sysbench
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
SB=$L/bin/sysbench
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
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
val() { awk -v k="$1" 'index($0, k) { sub(/^[^:]*: */, ""); print $1; exit }'; }
pos() { [ -n "$1" ] && awk -v v="$1" 'BEGIN {exit !(v + 0 > 0)}'; }
v=$("$SB" --version 2>&1); case "$v" in "sysbench "*) ok "--version: $v" ;; *) bad "--version: $v" ;; esac
r=$(to 60 "$SB" cpu --cpu-max-prime=2000 --time=1 run 2>&1); e=$(echo "$r" | val "events per second")
pos "$e" && ok "cpu: $e events/s" || { bad "cpu"; echo "$r" | tail -5; }
r=$(to 60 "$SB" memory --memory-block-size=1K --memory-total-size=16M --time=2 run 2>&1)
echo "$r" | awk '/MiB transferred/ {f=1} END {exit !f}' && ok "memory: $(echo "$r" | awk '/MiB transferred/ {print; exit}' | sed 's/^ *//')" || { bad "memory"; echo "$r" | tail -5; }
r=$(to 60 "$SB" threads --threads=2 --time=1 run 2>&1); e=$(echo "$r" | val "total number of events")
pos "$e" && ok "threads (2): $e events" || { bad "threads"; echo "$r" | tail -5; }
r=$(to 60 "$SB" mutex --threads=2 --mutex-loops=1000 run 2>&1); rc=$?
e=$(echo "$r" | awk '/events \(avg\/stddev\):/ {split($3, a, "/"); print a[1]; exit}')
[ $rc -eq 0 ] && pos "$e" && ok "mutex (2 threads): $e events per thread" || { bad "mutex (rc=$rc)"; echo "$r" | head -n 20; }
fo="--file-num=2 --file-total-size=4M"
to 60 "$SB" fileio $fo prepare > p.log 2>&1 && [ -f test_file.0 ] && [ -f test_file.1 ] && ok "fileio prepare: 2 files" || { bad "fileio prepare"; tail -3 p.log; }
r=$(to 60 "$SB" fileio $fo --file-test-mode=rndrw --time=1 run 2>&1); rc=$?
n=$(echo "$r" | awk '/^ *read: *IOPS=/ {sub(/.*IOPS=/, ""); print $1; exit}')
w=$(echo "$r" | awk '/^ *write: *IOPS=/ {sub(/.*IOPS=/, ""); print $1; exit}')
[ $rc -eq 0 ] && pos "$n" && pos "$w" && ok "fileio rndrw run: read IOPS $n, write IOPS $w" || { bad "fileio run (rc=$rc)"; echo "$r" | tail -n 8; }
echo "$r" | awk '/File operations:/ {f=1} f && NF {print "  info " $0} /fsyncs\/s/ {exit}' | head -n 4
to 60 "$SB" fileio $fo cleanup > c.log 2>&1 && [ ! -f test_file.0 ] && ok "fileio cleanup" || bad "fileio cleanup"
cat > t.lua <<'EOF'
function prepare()
  local t = {}
  for i = 1, 10 do t[#t + 1] = i * i end
  print(string.format("prep-ok %d %s", 6 * 7, table.concat(t, ",", 1, 3)))
end
function event()
  local x = 0
  for i = 1, 100 do x = x + i end
end
EOF
r=$("$SB" t.lua prepare 2>&1); case "$r" in *"prep-ok 42 1,4,9"*) ok "Lua script prepare hook: prep-ok 42 1,4,9" ;; *) bad "Lua prepare: $r" ;; esac
r=$(to 60 "$SB" t.lua --events=1000 --time=0 run 2>&1); e=$(echo "$r" | val "total number of events")
[ "$e" = 1000 ] && ok "Lua script run: exactly 1000 events" || { bad "Lua run: $e"; echo "$r" | tail -5; }
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
