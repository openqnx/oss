#!/bin/sh
# groff: format a man page (-man -Tascii), a document with a tbl table and
# an eqn equation, a -ms document to UTF-8 and to PostScript; nroff (the sh
# wrapper) formats too. The formatted text is checked for known content.
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
N=groff
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
GR=$L/bin/groff; PATH=$L/bin:$PATH; export PATH
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
info "$("$GR" --version 2>&1 | head -n 1)"
cat > q.1 <<'EOF'
.TH QNXDEMO 1 "2026" "QNX" "QNX Port Tests"
.SH NAME
qnxdemo \- demonstrate groff on QNX
.SH SYNOPSIS
.B qnxdemo
.RI [ options ]
.SH DESCRIPTION
The microkernel is called
.BR procnto (1).
EOF
r=$("$GR" -man -Tascii -P-cbou q.1 2>&1)
echo "$r" | has "QNXDEMO(1)" && echo "$r" | has "qnxdemo - demonstrate groff on QNX" && echo "$r" | has "procnto(1)" \
  && ok "-man -Tascii: header, NAME line, cross reference" || { bad "-man"; echo "$r" | head -8; }
echo "$r" | awk '/^NAME$/ {a=1} /^DESCRIPTION$/ {b=1} END {exit !(a && b)}' && ok "section headings NAME, DESCRIPTION" || bad "section headings"
cat > t.ms <<'EOF'
.TS
tab(;) allbox;
l n.
alpha;10
beta;200
.TE
.EQ
x = a + b
.EN
EOF
r=$("$GR" -t -e -Tascii -P-cbou t.ms 2>&1)
echo "$r" | awk '/alpha/ && /10/ {a=1} /beta/ && /200/ {b=1} /\+-+\+/ {c=1} END {exit !(a && b && c)}' && ok "tbl: boxed table rows" || { bad "tbl"; echo "$r" | head -8; }
echo "$r" | has "x=a+b" && ok "eqn: equation set (x=a+b)" || { bad "eqn"; echo "$r" | sed -n '/x/p' | head -3; }
printf '.TL\nQNX Paper\n.AU\nPort Tests\n.PP\nNeutrino is a microkernel.\n' > p.ms
r=$("$GR" -ms -Tutf8 -P-cbou p.ms 2>&1)
echo "$r" | has "QNX Paper" && echo "$r" | has "Neutrino is a microkernel." && ok "-ms -Tutf8: title and paragraph" || { bad "-ms utf8"; echo "$r" | head -5; }
"$GR" -ms -Tps p.ms > p.ps 2> ps.err && [ "$(head -c 11 p.ps)" = "%!PS-Adobe-" ] && has "QNX" < p.ps && ok "-Tps: PostScript ($(wc -c < p.ps | tr -d ' ') bytes)" || { bad "-Tps"; head -3 ps.err; }
r=$(printf '.ll 30\nThe quick brown fox jumps over the lazy dog near QNX.\n' | sh "$L/bin/nroff" 2>&1 | awk 'NF' | wc -l | tr -d ' ')
[ "$r" = 2 ] && ok "nroff: .ll 30 fills to 2 lines" || bad "nroff fill: $r lines"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
