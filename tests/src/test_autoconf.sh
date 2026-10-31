#!/bin/sh
# Autoconf: autoconf (perl + GNU m4) turns a configure.ac into a configure
# script, autoheader makes config.h.in, and the generated configure runs on
# QNX's sh: it substitutes variables into a file, defines into config.h and
# honours --with/--enable options. No compiler is involved.
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
N=autoconf
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
for p in autoconf autoheader autom4te; do
  h=$(head -n 1 "$L/bin/$p"); case "$h" in *"$L/bin/perl"*|*/usr/local/bin/perl*) ;; *) bad "$p starts with '$h', not the installed perl" ;; esac
done
v=$(to 30 "$L/bin/autoconf" --version 2>&1 | head -n 1); case "$v" in "autoconf (GNU Autoconf) "*) ok "autoconf --version (its own #! line): $v" ;; *) bad "autoconf --version: $v" ;; esac
AUTOCONF="to 60 $L/bin/autoconf"; AUTOHEADER="to 60 $L/bin/autoheader"
cat > configure.ac <<'EOF'
AC_INIT([qnxdemo], [2.5.1], [qnx@example.invalid])
AC_CONFIG_HEADERS([config.h])
AC_ARG_WITH([flavour], [AS_HELP_STRING([--with-flavour=X], [flavour])], [flavour=$withval], [flavour=plain])
AC_ARG_ENABLE([turbo], [AS_HELP_STRING([--enable-turbo], [go fast])], [turbo=$enableval], [turbo=no])
AC_SUBST([FLAVOUR], [$flavour])
AS_IF([test "x$turbo" = xyes], [AC_DEFINE([HAVE_TURBO], [1], [Turbo mode])])
AC_DEFINE_UNQUOTED([FLAVOUR_NAME], ["$flavour"], [The flavour])
AC_PROG_AWK
AC_PROG_SED
AC_CONFIG_FILES([info.txt])
AC_OUTPUT
EOF
printf 'package=@PACKAGE_NAME@ version=@PACKAGE_VERSION@ flavour=@FLAVOUR@\n' > info.txt.in
$AUTOCONF > ac.log 2>&1 && [ -s configure ] && ok "autoconf generated configure ($(wc -l < configure | tr -d ' ') lines)" || { bad "autoconf"; head -10 ac.log; }
$AUTOHEADER > ah.log 2>&1 && awk '/#undef HAVE_TURBO/ {f=1} END {exit !f}' config.h.in && ok "autoheader: config.h.in has #undef HAVE_TURBO" || { bad "autoheader"; head -5 ah.log; }
r=$(to 30 sh ./configure --version 2>&1 | head -n 1)
[ "$r" = "qnxdemo configure 2.5.1" ] && ok "configure --version: $r" || bad "configure --version: $r"
to 60 sh ./configure --with-flavour=neutrino --enable-turbo > cf.log 2>&1 && ok "configure ran on this sh" || { bad "configure"; tail -15 config.log 2>/dev/null; }
[ "$(cat info.txt 2>/dev/null)" = "package=qnxdemo version=2.5.1 flavour=neutrino" ] && ok "info.txt substituted" || bad "info.txt: $(cat info.txt 2>/dev/null)"
awk '/^#define HAVE_TURBO 1/ {a=1} /^#define FLAVOUR_NAME "neutrino"/ {b=1} END {exit !(a && b)}' config.h 2>/dev/null && ok "config.h: HAVE_TURBO 1, FLAVOUR_NAME \"neutrino\"" || { bad "config.h"; cat config.h 2>/dev/null | head; }
to 60 sh ./configure > cf2.log 2>&1 && awk '/HAVE_TURBO/ && /#define/ {f=1} END {exit f}' config.h && [ "$(cat info.txt)" = "package=qnxdemo version=2.5.1 flavour=plain" ] && ok "defaults: no turbo, flavour=plain" || bad "configure defaults"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
