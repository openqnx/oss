#!/bin/sh
# xfce4-dev-tools: xdt-csource (GLib) turns a file with quotes, a tab and a
# newline into the expected C array, escapes and length (17u); the
# target's aclocal finds the XDT_* macros in the installed share/aclocal and
# a configure built from XDT_VERSION_INIT and XDT_CHECK_PACKAGE has the
# version from the version macros and finds glib-2.0 through pkgconf (GLIB_VERSION); a
# missing package stops configure with XDT's message; xdt-autogen
# (NOCONFIGURE=1, it requires make: the make port) bootstraps a small
# automake project.
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
N=xfce4_dev_tools
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
printf 'QNX "8"\tneutrino\n' > in.txt
# The output is checked in a file: the target's sh echo would expand the
# backslash escapes in it.
to 60 "$L/bin/xdt-csource" --static --name=qnx_data in.txt > out.c 2>&1; rc=$?
[ $rc -eq 0 ] && grep -qxF '/* automatically generated from in.txt */' out.c && grep -qxF 'static const char qnx_data[] =' out.c \
  && grep -qxF '  "QNX \"8\"\tneutrino\n"' out.c && grep -qxF 'static const unsigned qnx_data_length = 17u;' out.c \
  && ok "xdt-csource --static: the array, escaped quotes/tab/newline, qnx_data_length = 17u" || { bad "xdt-csource (rc $rc)"; cat out.c; }
# The pkgconf port installs pkgconf, not a pkg-config name: tell configure.
# (XDT_CHECK_PACKAGE needs PKG_PROG_PKG_CONFIG from pkg.m4: pkgconf-dev.)
PKG_CONFIG=$L/bin/pkgconf; export PKG_CONFIG
cat > configure.ac <<'EOA'
XDT_VERSION_INIT([4.21.7])
AC_INIT([qnxxdt], [xdt_version])
AC_SUBST([QMINOR], [xdt_version_minor()])
XDT_CHECK_PACKAGE([GLIB], [glib-2.0], [2.50.0])
AC_CONFIG_FILES([out.txt])
AC_OUTPUT
EOA
printf 'glib=@GLIB_VERSION@\nlibs=@GLIB_LIBS@\nversion=@PACKAGE_VERSION@ minor=@QMINOR@\n' > out.txt.in
r=$(to 120 aclocal 2>&1); rc=$?
[ $rc -eq 0 ] && grep -q 'AC_DEFUN(\[XDT_CHECK_PACKAGE\]' aclocal.m4 2>/dev/null && ok "aclocal copied XDT_* macros from share/aclocal/xdt-*.m4" || bad "aclocal (rc $rc): $r"
r=$(to 120 autoconf 2>&1); rc=$?
[ $rc -eq 0 ] && ! grep -q 'XDT_CHECK_PACKAGE' configure && ok "autoconf: XDT_VERSION_INIT and XDT_CHECK_PACKAGE expanded" || bad "autoconf (rc $rc): $r"
r=$(to 120 sh ./configure 2>&1); rc=$?
g=$(sed -n 's/^glib=//p' out.txt 2>/dev/null)
[ $rc -eq 0 ] && case "$g" in 2.*) true ;; *) false ;; esac && grep -q '^libs=.*-lglib-2.0' out.txt && ok "XDT_CHECK_PACKAGE: glib-2.0 $g found by pkgconf"
grep -qx 'version=4.21.7 minor=21' out.txt && ok "XDT_VERSION_INIT: version 4.21.7, minor 21 (m4 esyscmd through the target's sh)" || bad "version: $(grep version= out.txt)" || { bad "configure (rc $rc): $(cat out.txt 2>/dev/null)"; echo "$r" | tail -n 4; }
sed 's/glib-2.0], \[2.50.0/no-such-package-qnx], [1.0/' configure.ac > c2.ac && mkdir c2 && mv c2.ac c2/configure.ac && cp out.txt.in c2/
r=$(cd c2 && aclocal >/dev/null 2>&1 && autoconf >/dev/null 2>&1 && to 120 sh ./configure 2>&1); rc=$?
[ $rc -ne 0 ] && echo "$r" | has "no-such-package-qnx" && ok "a missing package: configure stops naming no-such-package-qnx (exit $rc)" || bad "missing package (rc $rc): $(echo "$r" | tail -n 3)"
mkdir proj && cd proj || exit 1
printf 'AC_INIT([qnxproj], [0.1])\nAM_INIT_AUTOMAKE([foreign])\nAC_CONFIG_FILES([Makefile])\nAC_OUTPUT\n' > configure.ac
printf 'dist_data_DATA = hello.txt\n' > Makefile.am; echo hello > hello.txt
r=$(NOCONFIGURE=1 to 300 sh "$L/bin/xdt-autogen" 2>&1); rc=$?
[ $rc -eq 0 ] && [ -x configure ] && [ -f Makefile.in ] && ok "xdt-autogen: configure and Makefile.in generated" || { bad "xdt-autogen (rc $rc)"; echo "$r" | tail -n 8; }
cd "$T"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
