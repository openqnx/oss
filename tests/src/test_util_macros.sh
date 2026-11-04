#!/bin/sh
# util-macros: the target's aclocal finds xorg-macros.m4 in the installed
# share/aclocal and autoconf builds a configure from XORG_MACROS_VERSION,
# XORG_RELEASE_VERSION, XORG_ENABLE_DOCS and XORG_WITH_PERL; running it
# defines PACKAGE_VERSION_MAJOR/MINOR/PATCHLEVEL from the version (3.14.15),
# finds the target's perl and turns documentation off on request;
# requiring a newer util-macros (1.99) stops aclocal/autoconf with its
# message;
# pkgconf reports xorg-macros 1.20.2.
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
N=util_macros
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
AC_INIT([qnxtest], [3.14.15])
XORG_MACROS_VERSION(1.20)
XORG_RELEASE_VERSION
XORG_ENABLE_DOCS
XORG_WITH_PERL
AC_CONFIG_FILES([out.txt])
AC_OUTPUT
EOA
printf 'DEFS=@DEFS@\nperl=@PERL@\n' > out.txt.in
r=$(to 120 aclocal 2>&1); rc=$?
[ $rc -eq 0 ] && grep -q 'xorg-macros.m4' aclocal.m4 2>/dev/null && grep -q 'AC_DEFUN(\[XORG_RELEASE_VERSION\]' aclocal.m4 && ok "aclocal copied the XORG_* macros from share/aclocal/xorg-macros.m4" || bad "aclocal (rc $rc): $r"
r=$(to 120 autoconf 2>&1); rc=$?
[ $rc -eq 0 ] && [ -s configure ] && ! grep -q 'XORG_RELEASE_VERSION' configure && ok "autoconf: every XORG_* macro expanded" || bad "autoconf (rc $rc): $r"
r=$(to 120 sh ./configure --disable-docs 2>&1); rc=$?
d=$(sed -n 's/^DEFS=//p' out.txt 2>/dev/null)
[ $rc -eq 0 ] && echo "$d" | has "-DPACKAGE_VERSION_MAJOR=3" && echo "$d" | has "-DPACKAGE_VERSION_MINOR=14" && echo "$d" | has "-DPACKAGE_VERSION_PATCHLEVEL=15" \
  && ok "configure: PACKAGE_VERSION_MAJOR=3 MINOR=14 PATCHLEVEL=15" || { bad "configure (rc $rc): $d"; echo "$r" | tail -n 5; }
grep -q '^checking whether to build documentation\.\.\. no$' config.log 2>/dev/null || echo "$r" | grep -q '^checking whether to build documentation\.\.\. no$' \
  && ok "XORG_ENABLE_DOCS: --disable-docs honoured" || bad "docs: $(echo "$r" | grep documentation)"
p=$(sed -n 's/^perl=//p' out.txt); case "$p" in */perl) ok "XORG_WITH_PERL found $p" ;; *) bad "perl: $p" ;; esac
mkdir new && sed 's/XORG_MACROS_VERSION(1.20)/XORG_MACROS_VERSION(1.99)/' configure.ac > new/configure.ac && cp out.txt.in new/
r=$(cd new && { aclocal && autoconf; } 2>&1); rc=$?
[ $rc -ne 0 ] && echo "$r" | has "xorg-macros version 1.99 or higher is required but 1.20.2 found" && ok "XORG_MACROS_VERSION(1.99): stops with 'version 1.99 or higher is required but 1.20.2 found'" || bad "1.99 (rc $rc): $(echo "$r" | tail -n 3)"
r=$(pkgconf --modversion xorg-macros 2>&1 || pkg-config --modversion xorg-macros 2>&1)
[ "$r" = 1.20.2 ] && ok "pkgconf: xorg-macros 1.20.2" || bad "pkgconf xorg-macros: $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
