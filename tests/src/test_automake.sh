#!/bin/sh
# Automake: aclocal + automake --add-missing turn a Makefile.am (a script, a
# data file, a test) into Makefile.in; autoconf makes configure; the generated
# build then runs on the target -- configure, make (generates the script),
# make check (the parallel test harness) and make install DESTDIR=.
# No compiler is involved.
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
N=automake
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
PERL=$L/bin/perl; command -v perl >/dev/null 2>&1 && [ ! -x "$PERL" ] && PERL=$(command -v perl)
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
# Earlier runs hung for good in perl (a process blocked on a mutex). These
# probes, 20 s each, name the perl feature that blocks before autoconf runs.
pp() { r=$(to 20 "$PERL" -e "$2" 2>&1); rc=$?
  [ $rc -eq 0 ] && [ "$r" = ok ] && ok "perl: $1" || bad "perl: $1 (rc=$rc${r:+: $(echo "$r" | head -n 1)})"; }
pp "runs" 'print "ok"'
pp "backticks (fork + exec + pipe)" 'my $x = `echo ok`; chomp $x; print $x'
pp "system()" 'system("true") == 0 and print "ok"'
pp "fork + waitpid" 'my $p = fork; exit 0 unless $p; waitpid($p, 0); print "ok"'
pp "open -| (pipe from a child)" 'open(my $f, "-|", "echo", "ok") or die; my $x = <$f>; chomp $x; print $x'
pp "flock LOCK_EX" 'use Fcntl ":flock"; open(F, ">", "lock.tmp") or die; flock(F, LOCK_EX) or die "$!"; print "ok"'
pp "%ENV to a child" '$ENV{QNXP} = "ok"; my $x = qx(sh -c "echo \$QNXP"); chomp $x; print $x'
pp "signal names (%SIG)" 'local $SIG{HUP} = sub {}; local $SIG{TERM} = "DEFAULT"; print "ok"'
pp "%! errno names" 'mkdir "d.tmp"; mkdir "d.tmp" or ($!{EEXIST} and print "ok"); rmdir "d.tmp"'
pp "POSIX setlocale" 'use POSIX qw(setlocale LC_ALL); setlocale(LC_ALL, ""); print "ok"'
pp "File::Temp" 'use File::Temp qw(tempfile); my ($fh, $n) = tempfile(DIR => "."); unlink $n; print "ok"'
for p in aclocal automake autoconf autom4te; do
  h=$(head -n 1 "$L/bin/$p"); case "$h" in *"$L/bin/perl"*|*/usr/local/bin/perl*) ;; *) bad "$p starts with '$h', not the installed perl" ;; esac
done
PATH=$L/bin:$PATH; export PATH
info "$(to 30 automake --version 2>&1 | head -n 1)"
mkdir p && cd p
cat > configure.ac <<'EOF'
AC_INIT([qnxhello], [1.0])
AM_INIT_AUTOMAKE([foreign no-dependencies])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
EOF
cat > Makefile.am <<'EOF'
bin_SCRIPTS = qnxhello
CLEANFILES = qnxhello
dist_pkgdata_DATA = greeting.txt
EXTRA_DIST = qnxhello.in
qnxhello: qnxhello.in
	sed 's|@GREETING@|$(pkgdatadir)/greeting.txt|' $(srcdir)/qnxhello.in > $@
	chmod +x $@
TESTS = check-greeting.sh
dist_check_SCRIPTS = check-greeting.sh
EOF
printf '#!/bin/sh\ncat "@GREETING@"\n' > qnxhello.in
echo "hello from automake on QNX" > greeting.txt
printf '#!/bin/sh\n[ "$(cat $srcdir/greeting.txt)" = "hello from automake on QNX" ]\n' > check-greeting.sh; chmod +x check-greeting.sh
to 60 aclocal > al.log 2>&1 && [ -f aclocal.m4 ] && ok "aclocal: aclocal.m4" || { bad "aclocal"; head -5 al.log; }
to 60 automake --add-missing --copy > am.log 2>&1 && [ -f Makefile.in ] && [ -f install-sh ] && [ -f test-driver ] && ok "automake --add-missing: Makefile.in, install-sh, test-driver" || { bad "automake"; head -8 am.log; }
awk '/^install-binSCRIPTS:/ {a=1} /^check-TESTS:/ {b=1} END {exit !(a && b)}' Makefile.in 2>/dev/null && ok "Makefile.in has install-binSCRIPTS and check-TESTS" || bad "Makefile.in rules"
to 60 autoconf > ac.log 2>&1 && [ -s configure ] && ok "autoconf: configure" || { bad "autoconf"; head -5 ac.log; }
to 60 sh ./configure --prefix=/opt/qnxhello > cf.log 2>&1 && [ -f Makefile ] && ok "configure: Makefile generated" || { bad "configure"; tail -15 config.log 2>/dev/null; }
MK=$L/bin/make; [ -x "$MK" ] || MK=make
to 60 "$MK" > mk.log 2>&1 && [ -x qnxhello ] && ok "make: qnxhello generated" || { bad "make"; tail -5 mk.log; }
to 60 "$MK" check > ck.log 2>&1 && awk '/^# PASS: *1/ {f=1} END {exit !f}' ck.log && ok "make check: 1 test passed (parallel harness)" || { bad "make check"; tail -12 ck.log; }
to 60 "$MK" install DESTDIR="$T/inst" > in.log 2>&1 && [ -x "$T/inst/opt/qnxhello/bin/qnxhello" ] && [ -f "$T/inst/opt/qnxhello/share/qnxhello/greeting.txt" ] \
  && ok "make install DESTDIR=: script and data installed" || { bad "make install"; tail -5 in.log; }
r=$(sed 's|/opt/qnxhello|'"$T/inst/opt/qnxhello"'|' "$T/inst/opt/qnxhello/bin/qnxhello" 2>/dev/null | sh 2>&1)
[ "$r" = "hello from automake on QNX" ] && ok "installed script prints the greeting" || bad "installed script: $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
