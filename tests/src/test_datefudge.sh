#!/bin/sh
# datefudge: test_datefudge_clock (a program reading time(),
# gettimeofday() and clock_gettime(CLOCK_REALTIME)) run under datefudge
# 2001-02-03 04:05:06 UTC sees 981173106 (+ a second or two) from all three,
# CLOCK_MONOTONIC unaffected; with --static the time stands still at
# exactly 981173106; without datefudge it sees the real time; the
# target's date (toybox) under datefudge prints 2001-02-03; a bad date is
# refused. (QNX's runtime linker interposes the LD_PRELOADed library on the
# program's own calls, as on Linux.)
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
N=datefudge
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
D=$(cd "$(dirname "$0")" && pwd)   # helpers next to this test
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
DF=$L/bin/datefudge
r=$("$DF" --version 2>&1); echo "$r" | has "Version 1.24" && ok "datefudge --version: 1.24" || bad "version: $r"
HLP=$D/test_datefudge_clock
[ -x "$HLP" ] || bad "test_datefudge_clock not found next to the test"
now=$("$HLP" x | awk '{print $1}')
[ "$now" -gt 1700000000 ] 2>/dev/null && ok "without datefudge: the real time ($now)" || bad "real time: $now"
r=$(TZ=UTC "$DF" "2001-02-03 04:05:06" "$HLP" x 2>&1)
set -- $r
in_() { [ "$1" -ge 981173106 ] 2>/dev/null && [ "$1" -le 981173110 ]; }
in_ "$1" && in_ "$2" && in_ "$3" && [ "$4" = mono-ok ] && ok "fudged to 2001-02-03 04:05:06 UTC: time() $1, gettimeofday $2, clock_gettime $3; monotonic clock runs" || bad "fudged: $r"
r=$(TZ=UTC "$DF" --static "2001-02-03 04:05:06" "$HLP" x 2>&1)
[ "$r" = "981173106 981173106 981173106 mono-ok" ] && ok "--static: exactly 981173106 from all three" || bad "--static: $r"
r=$(TZ=UTC "$DF" "2001-02-03 04:05:06" date -u +%Y-%m-%d 2>&1)
[ "$r" = "2001-02-03" ] && ok "the target's date under datefudge: 2001-02-03" || bad "date under datefudge: $r"
r=$("$DF" "not a date at all" "$HLP" x 2>&1); rc=$?
[ $rc -ne 0 ] && ok "an unparsable date is refused (exit $rc)" || bad "bad date accepted: $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
