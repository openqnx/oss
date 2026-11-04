#!/bin/sh
# lmbench micro-benchmarks, short runs: system call, read/write and stat
# latency, pipe latency and bandwidth, memory read bandwidth, process
# creation, and TCP latency over loopback against lat_tcp's own server.
# Each must produce its result line with a positive number.
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
N=lmbench
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
B=$L/lib/lmbench/bin
# Short runs that still measure: skip lmbench's timer/loop-overhead
# calibration (TIMING_O/LOOP_O, seconds of work on an emulated CPU) and time
# each sample for 20 ms (ENOUGH, microseconds).
TIMING_O=0; LOOP_O=0; ENOUGH=20000; export TIMING_O LOOP_O ENOUGH
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
# run <description> <label printed by the benchmark> <command...>: the line
# "<label>: <number> ..." must appear with number > 0.
run() {
  d=$1; lab=$2; shift 2
  r=$(to 30 "$@" 2>&1)
  v=$(echo "$r" | awk -v l="$lab" 'index($0, l ":") { sub(/^[^:]*: */, ""); print $1; exit }')
  if [ -n "$v" ] && awk -v v="$v" 'BEGIN {exit !(v + 0 > 0)}'; then ok "$d: $(echo "$r" | awk -v l="$lab" 'index($0, l ":") {print; exit}')"
  else bad "$d: $(echo "$r" | head -n 2)"; fi
}
r=$("$B/hello" 2>&1); [ "$r" = "Hello world" ] && ok "hello: Hello world" || bad "hello: $r"
run "lat_syscall null" "Simple syscall" "$B/lat_syscall" -N 1 null
run "lat_syscall read" "Simple read" "$B/lat_syscall" -N 1 read
run "lat_syscall write" "Simple write" "$B/lat_syscall" -N 1 write
echo x > f; run "lat_syscall stat" "Simple stat" "$B/lat_syscall" -N 1 stat "$T/f"
run "lat_pipe" "Pipe latency" "$B/lat_pipe" -N 1
run "bw_pipe" "Pipe bandwidth" "$B/bw_pipe" -N 1
run "lat_proc fork" "Process fork+exit" "$B/lat_proc" -N 1 fork
r=$(to 30 "$B/bw_mem" -N 1 256k rd 2>&1)
echo "$r" | awk 'NF == 2 && $1 + 0 > 0 && $2 + 0 > 0 {f=1} END {exit !f}' && ok "bw_mem 256k rd: $r MB/s" || bad "bw_mem: $r"
qstop_hook() { "$B/lat_tcp" -S 127.0.0.1 > /dev/null 2>&1; }
to 20 "$B/lat_tcp" -s > srv.log 2>&1 < /dev/null; sleep 2
run "lat_tcp over loopback" "TCP latency using 127.0.0.1" "$B/lat_tcp" -N 1 127.0.0.1
to 20 "$B/lat_tcp" -S 127.0.0.1 > /dev/null 2>&1; qstop_hook() { :; }
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
