#!/bin/sh
# help2man (a Perl script, run by the target's perl): a man page made from
# a sample program's --help and --version output, with SOURCE_DATE_EPOCH=0,
# is byte-identical to what the same help2man 1.49.3 makes on the build
# machine (known checksums): .TH from --version, NAME, the SYNOPSIS from
# the usage line, the options as .TP entries, REPORTING BUGS, COPYRIGHT;
# -n sets the NAME description and -i includes an [EXAMPLES] section; a
# program that cannot be run is an error.
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
N=help2man
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
H2=$L/bin/help2man
r=$("$H2" --version 2>&1 | head -n 1); [ "$r" = "GNU help2man 1.49.3" ] && ok "help2man --version: 1.49.3" || bad "version: $r"
cat > qnxhello <<'EOQ'
#!/bin/sh
case "$1" in
--help) cat <<'EOH'
Usage: qnxhello [OPTION]... [NAME]
Greet the QNX world.

  -n, --name=NAME   greet NAME instead of the world
  -l, --loud        greet in capitals
      --help        display this help and exit
      --version     output version information and exit

Report bugs to nobody.
EOH
;;
--version) printf 'qnxhello 2.5\nCopyright (C) 2026 Nobody.\n' ;;
*) echo "hello, ${1:-world}" ;;
esac
EOQ
chmod 755 qnxhello
SOURCE_DATE_EPOCH=0 to 60 "$H2" -N ./qnxhello > q.1 2> err.txt; rc=$?
c=$(cksum < q.1 | awk '{print $1, $2}')
[ $rc -eq 0 ] && [ "$c" = "1065383771 596" ] && ok "man page identical to the build machine's help2man output (cksum $c)" || { bad "q.1 (rc $rc, cksum $c): $(cat err.txt)"; cat q.1; }
grep -qF '.TH QNXHELLO "1" "January 1970" "qnxhello 2.5" "User Commands"' q.1 && ok ".TH: name, section, date, version" || bad ".TH: $(grep '^.TH' q.1)"
grep -qF '\fB\-n\fR, \fB\-\-name\fR=\fI\,NAME\/\fR' q.1 && ok "option -n, --name=NAME as a .TP entry" || bad "options"
printf '[EXAMPLES]\n.B qnxhello\nQNX\n' > extra.h2m
SOURCE_DATE_EPOCH=0 to 60 "$H2" -N -n "greet the QNX world" -i extra.h2m ./qnxhello > q2.1 2> err.txt; rc=$?
c=$(cksum < q2.1 | awk '{print $1, $2}')
[ $rc -eq 0 ] && [ "$c" = "2290595271 616" ] && grep -qF 'qnxhello \- greet the QNX world' q2.1 && grep -q '^\.SH EXAMPLES$' q2.1 \
  && ok "-n description and -i [EXAMPLES] include (cksum $c)" || { bad "q2.1 (rc $rc, cksum $c): $(cat err.txt)"; }
r=$(to 60 "$H2" ./nonexist 2>&1); rc=$?
[ $rc -ne 0 ] && echo "$r" | has "can't get \`--help' info from ./nonexist" && ok "a program that cannot run: error (exit $rc)" || bad "nonexist (rc $rc): $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
