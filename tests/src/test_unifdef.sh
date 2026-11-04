#!/bin/sh
# unifdef: resolves the conditionals of a sample C file for given symbols
# (#ifdef/#else, #if defined() && comparison, #elif, #ifndef of an unknown
# symbol left alone) with exactly the expected output and exit status 1
# ("changed"); -s lists the symbols the conditionals use; an unchanged run
# exits 0; an unterminated #if is an error (exit 2); symbols read from a
# file of #define/#undef lines (-f). (unifdefall is not tested: it runs
# "cc -E -dM", and a QNX target has no C compiler.)
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
N=unifdef
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
U=$L/bin/unifdef
cat > in.c <<'EOI'
#include <stdio.h>
#ifdef QNX
int os(void) { return 8; }
#else
int os(void) { return 0; }
#endif
#if defined(DEBUG) && LEVEL > 2
#define TRACE 1
#elif VERBOSE
#define TRACE 2
#endif
#ifndef UNKNOWN
int keep;
#endif
EOI
r=$("$U" -DQNX -UDEBUG in.c 2>&1); rc=$?
exp='#include <stdio.h>
int os(void) { return 8; }
#if   VERBOSE
#define TRACE 2
#endif
#ifndef UNKNOWN
int keep;
#endif'
[ $rc -eq 1 ] && [ "$r" = "$exp" ] && ok "-DQNX -UDEBUG: #ifdef taken, #if/#elif reduced to the #elif VERBOSE branch, exit 1" || { bad "-DQNX -UDEBUG (rc $rc)"; echo "$r"; }
r=$("$U" -UQNX -DDEBUG -DLEVEL=3 in.c 2>&1); rc=$?
exp='#include <stdio.h>
int os(void) { return 0; }
#define TRACE 1
#ifndef UNKNOWN
int keep;
#endif'
[ $rc -eq 1 ] && [ "$r" = "$exp" ] && ok "-UQNX -DDEBUG -DLEVEL=3: #else branch, defined(DEBUG) && LEVEL > 2 true" || { bad "-UQNX -DDEBUG -DLEVEL=3 (rc $rc)"; echo "$r"; }
r=$("$U" -s in.c 2>&1 | tr '\n' ' ')
[ "$r" = "QNX DEBUG LEVEL VERBOSE UNKNOWN " ] && ok "-s: the five symbols in order" || bad "-s: $r"
"$U" -DOTHER in.c > same.c; rc=$?
[ $rc -eq 0 ] && cmp -s in.c same.c && ok "a symbol the file does not use: output unchanged, exit 0" || bad "-DOTHER: rc $rc"
printf '#if 1\nx\n' > bad.c
r=$("$U" -DX bad.c 2>&1); rc=$?
[ $rc -eq 2 ] && echo "$r" | has "bad.c: 3: Premature EOF (#if line 1 depth 1)" && ok "unterminated #if: Premature EOF, exit 2" || bad "bad.c (rc $rc): $r"
printf '#define A 1\n#if A\nyes\n#else\nno\n#endif\n#ifdef B\nb\n#endif\n' > all.c
printf '#define A 1\n#undef B\n' > defs.h
r=$("$U" -f defs.h all.c 2>&1)
[ "$r" = "#define A 1
yes" ] && ok "-f defs.h: A defined, B undefined, both conditionals resolved" || { bad "-f defs.h"; echo "$r"; }
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
