#!/bin/sh
# flutter-counter-example (Flutter's counter starter app for QNX): the bundle
# is complete (AOT libapp.so for this CPU, assets with the Material icons
# font), the menu entry starts flutter-counter, and flutter-counter itself --
# with an empty LD_LIBRARY_PATH, as from a plain shell -- runs the app on a
# headless labwc compositor and stops cleanly on SIGTERM.
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
N=flutter_counter_example
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
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL flutter_counter_example (qnxport-timeout.sh missing)"; exit 1; }
APP=$L/share/flutter-apps/counter; C=$L/bin/flutter-counter; DF=$L/share/applications/flutter-counter.desktop
[ -x "$C" ] && [ -f "$APP/libapp.so" ] && [ -d "$APP/flutter_assets" ] && ok "flutter-counter and the app bundle installed" || bad "not installed ($C, $APP)"
# ELF e_machine (offset 18): 0x3e x86_64, 0xb7 aarch64 -- the CPU this runs on
m=$(od -An -tx1 -j18 -N1 "$APP/libapp.so" 2>/dev/null | tr -d ' ')
# QNX: uname -p is the CPU (x86_64, aarch64le); uname -m the board
case "$(uname -p)" in x86_64*) want=3e ;; aarch64*) want=b7 ;; *) want="" ;; esac
[ -n "$want" ] && [ "$m" = "$want" ] && ok "libapp.so is AOT code for $(uname -p) (e_machine 0x$m)" || bad "libapp.so e_machine 0x$m on $(uname -p)"
has "MaterialIcons" < "$APP/flutter_assets/FontManifest.json" && [ -f "$APP/flutter_assets/fonts/MaterialIcons-Regular.otf" ] \
  && ok "assets: Material icons font listed and present" || bad "Material icons font missing from the assets"
if [ -f "$DF" ]; then
  has "Exec=flutter-counter" < "$DF" && has "Type=Application" < "$DF" && has "Name=Flutter Counter" < "$DF" \
    && ok "menu entry: Flutter Counter, Exec=flutter-counter" || bad "menu entry: $(cat "$DF")"
else bad "no $DF"; fi
RT=$T/rt; mkdir -m 700 "$RT"
XDG_RUNTIME_DIR=$RT WLR_BACKENDS=headless WLR_RENDERER=pixman "$L/bin/labwc" > labwc.log 2>&1 &
track $!
i=0; while [ ! -S "$RT/wayland-0" ] && [ $i -lt 150 ]; do sleep 0.1; i=$((i+1)); done
[ -S "$RT/wayland-0" ] && ok "labwc up (headless)" || { bad "labwc"; cat labwc.log; }
( unset LD_LIBRARY_PATH; XDG_RUNTIME_DIR=$RT WAYLAND_DISPLAY=wayland-0 exec "$C" ) > app.out 2>&1 &
ap=$!; track $ap
i=0; while ! has "running" < app.out && [ $i -lt 60 ] && kill -0 $ap 2>/dev/null; do sleep 1; i=$((i+1)); done
has "running $APP" < app.out && ok "flutter-counter runs the app without LD_LIBRARY_PATH: $(head -n 1 app.out)" || { bad "no 'running' line after ${i} s"; cat app.out; }
sleep 3
kill -0 $ap 2>/dev/null && ok "still running 3 s later" || { bad "the app ended"; cat app.out; }
kill -TERM $ap 2>/dev/null; i=0; while kill -0 $ap 2>/dev/null && [ $i -lt 10 ]; do sleep 1; i=$((i+1)); done
if kill -0 $ap 2>/dev/null; then bad "still running 10 s after SIGTERM"; else wait $ap; rc=$?
  [ $rc -eq 0 ] && ok "SIGTERM: clean exit (rc 0)" || bad "exit status $rc after SIGTERM"; fi
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
