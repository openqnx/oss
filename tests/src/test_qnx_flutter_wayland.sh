#!/bin/sh
# qnx-flutter-wayland (the Flutter embedder for QNX's Wayland desktop): --help
# names the bundle layout; a bundle that is not there is refused with a
# message; then, against a headless labwc compositor (pixman), it runs the
# counter example's bundle (flutter-counter-example): it reports the running
# app and its window size, keeps running, and stops cleanly on SIGTERM.
# (Clicks and pixels on a real screen are checked on the graphics images.)
# Runs in the background with its output to a log, for at most 170 s (a hang
# ends as a FAIL with what it printed so far).
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
N=qnx_flutter_wayland
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
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL qnx_flutter_wayland (qnxport-timeout.sh missing)"; exit 1; }
W=$L/bin/qnx-flutter-wayland; APP=$L/share/flutter-apps/counter
export LD_LIBRARY_PATH=$L/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
h=$("$W" --help 2>&1); rc=$?
[ $rc -eq 0 ] && echo "$h" | has "<bundle dir>" && echo "$h" | has "libapp.so" && echo "$h" | has "flutter_assets" \
  && ok "--help: usage with the bundle layout" || { bad "--help (rc $rc)"; echo "$h"; }
"$W" "$T/no-such-bundle" > nob.out 2>&1; rc=$?
[ $rc -ne 0 ] && { has "cannot" < nob.out || has "Wayland" < nob.out; } && ok "a missing bundle (or no display) is refused (rc $rc)" \
  || { bad "missing bundle: rc $rc"; cat nob.out; }
if [ ! -f "$APP/libapp.so" ]; then
  info "flutter-counter-example not installed: no app run"
else
  RT=$T/rt; mkdir -m 700 "$RT"
  XDG_RUNTIME_DIR=$RT WLR_BACKENDS=headless WLR_RENDERER=pixman "$L/bin/labwc" > labwc.log 2>&1 &
  track $!
  i=0; while [ ! -S "$RT/wayland-0" ] && [ $i -lt 150 ]; do sleep 0.1; i=$((i+1)); done
  [ -S "$RT/wayland-0" ] && ok "labwc up (headless)" || { bad "labwc"; cat labwc.log; }
  XDG_RUNTIME_DIR=$RT WAYLAND_DISPLAY=wayland-0 "$W" "$APP" > app.out 2>&1 &
  ap=$!; track $ap
  i=0; while ! has "running" < app.out && [ $i -lt 60 ] && kill -0 $ap 2>/dev/null; do sleep 1; i=$((i+1)); done
  has "qnx-flutter-wayland: running $APP" < app.out && ok "the app runs: $(head -n 1 app.out)" || { bad "no 'running' line after ${i} s"; cat app.out; }
  sleep 5
  kill -0 $ap 2>/dev/null && ok "still running 5 s later (frames on the headless output)" || { bad "the app ended"; cat app.out; }
  kill -TERM $ap 2>/dev/null; i=0; while kill -0 $ap 2>/dev/null && [ $i -lt 10 ]; do sleep 1; i=$((i+1)); done
  if kill -0 $ap 2>/dev/null; then bad "still running 10 s after SIGTERM"; else wait $ap; rc=$?
    [ $rc -eq 0 ] && ok "SIGTERM: clean exit (rc 0) after ${i} s" || bad "exit status $rc after SIGTERM"; fi
fi
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
