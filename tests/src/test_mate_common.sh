#!/bin/sh
# mate-common: the target's aclocal finds mate-common's macros in the
# installed share/aclocal; a configure built from AM_MAINTAINER_MODE and
# MATE_MAINTAINER_MODE_DEFINES sets DISABLE_DEPRECATED to the -D*_DISABLE_
# DEPRECATED flags in maintainer mode and to nothing without it;
# mate-autogen (NOCONFIGURE=1) bootstraps a small automake project with the
# target's autoreconf/automake, giving configure and Makefile.in.
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
N=mate_common
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
export PATH="$L/bin:$PATH"
cat > configure.ac <<'EOA'
AC_INIT([qnxmate], [1.0])
AM_MAINTAINER_MODE([enable])
MATE_MAINTAINER_MODE_DEFINES
AC_CONFIG_FILES([out.txt])
AC_OUTPUT
EOA
printf 'dd=@DISABLE_DEPRECATED@\n' > out.txt.in
r=$(to 120 aclocal 2>&1); rc=$?
[ $rc -eq 0 ] && grep -q 'mate-common.m4' aclocal.m4 2>/dev/null && ok "aclocal copied MATE_* macros from share/aclocal/mate-common.m4" || bad "aclocal (rc $rc): $r"
r=$(to 120 autoconf 2>&1); rc=$?
[ $rc -eq 0 ] && ! grep -q 'MATE_MAINTAINER_MODE_DEFINES' configure && ok "autoconf: MATE_MAINTAINER_MODE_DEFINES expanded" || bad "autoconf (rc $rc): $r"
r=$(to 120 sh ./configure 2>&1); rc=$?
[ $rc -eq 0 ] && grep -q '^dd=.*-DGTK_DISABLE_DEPRECATED -DGTK_DISABLE_SINGLE_INCLUDES.*-DMATE_DISABLE_DEPRECATED' out.txt \
  && ok "maintainer mode: DISABLE_DEPRECATED has -DGTK_DISABLE_DEPRECATED ... -DMATE_DISABLE_DEPRECATED" || { bad "configure (rc $rc): $(cat out.txt 2>/dev/null)"; echo "$r" | tail -n 4; }
r=$(to 120 sh ./configure --disable-maintainer-mode 2>&1); rc=$?
[ $rc -eq 0 ] && [ "$(cat out.txt)" = "dd=" ] && ok "--disable-maintainer-mode: DISABLE_DEPRECATED empty" || bad "no maintainer mode (rc $rc): $(cat out.txt)"
mkdir proj && cd proj || exit 1
printf 'AC_INIT([qnxproj], [0.1])\nAM_INIT_AUTOMAKE([foreign])\nAC_CONFIG_FILES([Makefile])\nAC_OUTPUT\n' > configure.ac
printf 'dist_data_DATA = hello.txt\n' > Makefile.am; echo hello > hello.txt
r=$(NOCONFIGURE=1 to 300 sh "$L/bin/mate-autogen" 2>&1); rc=$?
[ $rc -eq 0 ] && [ -x configure ] && [ -f Makefile.in ] && grep -q 'dist_data_DATA = hello.txt' Makefile.in \
  && ok "mate-autogen: configure and Makefile.in generated" || { bad "mate-autogen (rc $rc)"; echo "$r" | tail -n 8; }
cd ..
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
