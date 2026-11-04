#!/bin/sh
# gperf: generate a perfect hash for a keyword set with a struct type --
# the C source must define the lookup function, the right keyword count and
# word lengths and every keyword; duplicate keys must be refused without -D
# and accepted with it.
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
N=gperf
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
fhas() { awk -v p="$2" 'index($0, p) {f=1} END {exit !f}' "$1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
GP=$L/bin/gperf
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
info "$("$GP" --version 2>&1 | head -n 1)"
cat > kw.gperf <<'EOF'
%{
#include <string.h>
%}
%language=ANSI-C
%struct-type
%define lookup-function-name qnx_lookup
%define hash-function-name qnx_hash
struct kw { const char *name; int id; };
%%
open, 1
close, 2
read, 3
write, 4
procnto, 5
devc-pty, 6
%%
EOF
"$GP" --output-file=kw.c kw.gperf 2> err.txt && [ -s kw.c ] && ok "generated kw.c ($(wc -l < kw.c | tr -d ' ') lines)" || { bad "gperf run"; head -5 err.txt; }
fhas kw.c "qnx_lookup (" && fhas kw.c "const char *str" && ok "lookup function qnx_lookup (ANSI-C prototype)" || bad "lookup function"
fhas kw.c "qnx_hash (" && ok "hash function qnx_hash" || bad "hash function"
fhas kw.c "TOTAL_KEYWORDS = 6" || fhas kw.c "#define TOTAL_KEYWORDS 6"; [ $? -eq 0 ] && ok "TOTAL_KEYWORDS 6" || bad "TOTAL_KEYWORDS"
fhas kw.c "MIN_WORD_LENGTH = 4" || fhas kw.c "#define MIN_WORD_LENGTH 4"; [ $? -eq 0 ] && ok "MIN_WORD_LENGTH 4" || bad "MIN_WORD_LENGTH"
fhas kw.c "MAX_WORD_LENGTH = 8" || fhas kw.c "#define MAX_WORD_LENGTH 8"; [ $? -eq 0 ] && ok "MAX_WORD_LENGTH 8" || bad "MAX_WORD_LENGTH"
m=0; for k in '{"open",' '{"close",' '{"read",' '{"write",' '{"procnto",' '{"devc-pty",'; do fhas kw.c "$k" || m=$((m+1)); done
[ $m -eq 0 ] && ok "all 6 keyword entries in the table" || bad "$m keyword entries missing"
printf 'alpha\nbeta\nalpha\n' > dup.gperf
"$GP" dup.gperf > /dev/null 2> dup.err && bad "duplicate key accepted without -D" || { fhas dup.err "use option -D" && ok "duplicate key refused without -D" || bad "duplicate refusal message: $(cat dup.err)"; }
"$GP" -D dup.gperf > dup.c 2>/dev/null && fhas dup.c '"alpha"' && ok "-D accepts duplicates" || bad "-D duplicates"
printf 'red\ngreen\nblue\n' | "$GP" -N is_colour > col.c 2>/dev/null && fhas col.c "is_colour" && fhas col.c '"green"' && ok "keywords from stdin, -N is_colour" || bad "stdin keywords"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
