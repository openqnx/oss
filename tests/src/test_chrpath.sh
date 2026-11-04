#!/bin/sh
# chrpath: on a copy of test_chrpath_prog (an ELF program of the target's
# architecture, linked with DT_RPATH /opt/qnx-chrpath-test/lib:/usr/local/lib)
# chrpath lists the path, replaces it with a shorter one, converts it to
# DT_RUNPATH (-c) and deletes it (-d); after each edit chrpath -l shows the
# new state and the program still loads and runs; a path longer than the
# old one is refused; a file that is not ELF is reported.
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
N=chrpath
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
CP=$L/bin/chrpath
r=$("$CP" -v 2>&1); echo "$r" | has "chrpath version 0.18" && ok "chrpath -v: 0.18" || bad "version: $r"
[ -x "$D/test_chrpath_prog" ] || bad "test_chrpath_prog not found next to the test"
cp "$D/test_chrpath_prog" p && chmod 755 p
# After a program exits, QNX keeps its file busy for a moment: opening it for
# writing (which chrpath does) fails with EBUSY ("Resource busy"). Each run
# is therefore followed by replacing p with a copy of itself.
runs() { ./p > run.log 2>&1 && has "PASS chrpath_prog" < run.log; rc_=$?; cp p p.tmp && mv -f p.tmp p; return $rc_; }
r=$("$CP" -l p 2>&1)
[ "$r" = "p: RPATH=/opt/qnx-chrpath-test/lib:/usr/local/lib" ] && ok "-l: RPATH=/opt/qnx-chrpath-test/lib:/usr/local/lib" || bad "-l: $r"
r=$("$CP" -r /usr/local/lib p 2>&1); rc=$?
[ $rc -eq 0 ] && echo "$r" | has "p: new RPATH: /usr/local/lib" && ok "-r /usr/local/lib: replaced" || bad "-r (rc $rc): $r"
r=$("$CP" -l p 2>&1); [ "$r" = "p: RPATH=/usr/local/lib" ] && ok "-l after -r: RPATH=/usr/local/lib" || bad "-l after -r: $r"
runs && ok "the edited program loads and runs" || { bad "program after -r"; cat run.log; }
r=$("$CP" -r /a/path/that/is/much/longer/than/the/one/stored/in/the/file p 2>&1); rc=$?
[ $rc -ne 0 ] && echo "$r" | has "too large" && ok "a longer path is refused (exit $rc): $(echo "$r" | tail -n 1)" || bad "long -r (rc $rc): $r"
r=$("$CP" -c p 2>&1); rc=$?
r2=$("$CP" -l p 2>&1)
[ $rc -eq 0 ] && [ "$r2" = "p: RUNPATH=/usr/local/lib" ] && ok "-c: converted to RUNPATH" || bad "-c (rc $rc): $r / $r2"
runs && ok "the program runs with DT_RUNPATH" || { bad "program after -c"; cat run.log; }
r=$("$CP" -d p 2>&1); rc=$?
r2=$("$CP" -l p 2>&1); rc2=$?
[ $rc -eq 0 ] && [ $rc2 -ne 0 ] && echo "$r2" | has "no rpath or runpath tag found" && ok "-d: path deleted (-l: no rpath or runpath tag found)" || bad "-d (rc $rc/$rc2): $r / $r2"
runs && ok "the program runs without a path" || { bad "program after -d"; cat run.log; }
echo "not an elf file at all" > notelf
r=$("$CP" -l notelf 2>&1); rc=$?
[ $rc -ne 0 ] && ok "a non-ELF file is an error (exit $rc): $r" || bad "notelf (rc $rc): $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
