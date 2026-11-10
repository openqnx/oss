#!/bin/sh
# Qt Quick Effect Maker (qqem): the program starts (QGuiApplication on the
# offscreen platform) and answers --version and --help with its own text,
# and the effect nodes it composes effects from are installed where it looks
# for them (QmlImportsPath/QtQuickEffectMaker/defaultnodes), each a JSON
# node description (known names), with the precompiled blur shader pair.
# Its editor window is a Qt Quick application on a GPU: a screen test on a
# graphics target shows it.
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
N=qt6-quickeffectmaker
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
QT_QPA_PLATFORM=offscreen; export QT_QPA_PLATFORM
Q=$L/bin/qqem
[ -x "$Q" ] && ok "qqem installed" || bad "qqem installed"
v=$("$Q" --version 2>&1); info "$v"
case "$v" in *"Qt Quick Effect Maker 0.44"*) ok "--version: Qt Quick Effect Maker 0.44" ;; *) bad "--version" ;; esac
h=$("$Q" --help 2>&1)
case "$h" in *--exportpath*) case "$h" in *--create*) ok "--help lists --exportpath and --create" ;; *) bad "--help" ;; esac ;; *) bad "--help" ;; esac
D=$L/qml/QtQuickEffectMaker/defaultnodes
n=$(ls "$D"/basic/*.qen "$D"/common/*.qen "$D"/extra/*.qen "$D"/nature/*.qen 2>/dev/null | wc -l | tr -d ' ')
info "$n nodes in $D"
[ "$n" -ge 40 ] && ok "effect nodes installed (40+)" || bad "effect nodes installed"
grep -q '"name": "Vignette"' "$D/basic/vignette.qen" && grep -q '"name": "FastBlur"' "$D/basic/fastblur.qen" \
  && ok "node descriptions (Vignette, FastBlur)" || bad "node descriptions"
[ -s "$D/common/bluritems.frag.qsb" ] && [ -s "$D/common/bluritems.vert.qsb" ] && ok "blur shader pair (.qsb)" || bad "blur shader pair"
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
