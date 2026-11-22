#!/bin/sh
# Parole (Xfce media player, GTK 3 + GStreamer): --version names the
# release; --help lists its playback options; its plugins load (each is a
# shared library with every dependency resolved); the GStreamer it plays
# through decodes and plays a generated stream end to end (audiotestsrc ->
# audioconvert -> wavenc into a file, which then has a RIFF/WAVE header and
# the expected size) -- the pipeline Parole builds for audio, without a
# display. Parole is a GTK program: it reads its options only after GTK
# has a display, so a headless labwc (pixman renderer) is started for it.
# Parole reads its settings through xfconf, which needs a D-Bus session bus:
# a private one is started for the run (no dbus-launch/autolaunch here).
echo "== parole =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=${QNX_PORT_PREFIX:-/usr/local}
T=$(mktemp -d /tmp/qnxport-parole.XXXXXX) || { echo "FAIL parole (mktemp)"; exit 1; }
DBP=""
LWP=""
trap '[ -n "$DBP" ] && kill $DBP 2>/dev/null; [ -n "$LWP" ] && kill $LWP 2>/dev/null; cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

if [ -x "$P/bin/dbus-daemon" ]; then
  DBUS_SESSION_BUS_ADDRESS="unix:path=$T/bus"; export DBUS_SESSION_BUS_ADDRESS
  "$P/bin/dbus-daemon" --config-file="$P/share/dbus-1/session.conf" --address="$DBUS_SESSION_BUS_ADDRESS" --nofork --nopidfile > dbus.log 2>&1 &
  DBP=$!
  i=0; while [ $i -lt 10 ] && [ ! -S "$T/bus" ]; do sleep 1; i=$((i+1)); done
  [ -S "$T/bus" ] && ok "private D-Bus session bus" || bad "dbus-daemon: $(head -n 2 dbus.log)"
else bad "dbus-daemon not installed"; fi
RT=$T/rt; mkdir -m 700 "$RT"
XDG_RUNTIME_DIR=$RT WLR_BACKENDS=headless WLR_RENDERER=pixman "$P/bin/labwc" > labwc.log 2>&1 &
LWP=$!
i=0; while [ ! -S "$RT/wayland-0" ] && [ $i -lt 150 ]; do sleep 0.1; i=$((i+1)); done
[ -S "$RT/wayland-0" ] && ok "labwc up (headless)" || bad "labwc: $(tail -n 2 labwc.log)"
XDG_RUNTIME_DIR=$RT; WAYLAND_DISPLAY=wayland-0; GDK_BACKEND=wayland; export XDG_RUNTIME_DIR WAYLAND_DISPLAY GDK_BACKEND
v=$(timeout 60 "$P/bin/parole" --version 2>&1 | grep -m1 "Parole Media Player")
echo "  info $v"
case "$v" in *"4.20.0"*) ok "parole --version: 4.20.0" ;; *) bad "parole --version: $v" ;; esac
h=$(timeout 60 "$P/bin/parole" --help 2>&1)
echo "$h" | grep -q -- "--play" && echo "$h" | grep -q -- "--volume" && ok "parole --help lists --play and --volume" || bad "parole --help: $(echo "$h" | head -n 3)"

n=0; nb=0
for so in "$P"/lib/parole-0/*.so; do
  [ -f "$so" ] || continue
  n=$((n+1))
  ldd "$so" > ldd.out 2>&1 || { nb=$((nb+1)); echo "  info $so: $(head -n 2 ldd.out)"; }
done
[ $n -gt 0 ] && [ $nb -eq 0 ] && ok "$n Parole plugins load (ldd)" || bad "plugins: $n found, $nb do not load"

G=$P/bin/gst-launch-1.0
if [ -x "$G" ]; then
  timeout 60 "$G" -q audiotestsrc num-buffers=20 samplesperbuffer=1024 ! audio/x-raw,rate=44100,channels=2 ! audioconvert ! wavenc ! filesink location=t.wav > gst.out 2>&1
  sz=$(wc -c < t.wav 2>/dev/null | tr -d ' ')
  hd=$(head -c 12 t.wav 2>/dev/null | od -An -c | tr -d ' \n')
  echo "  info t.wav: ${sz:-0} bytes"
  case "$hd" in RIFF*WAVE) [ "${sz:-0}" -gt 80000 ] && ok "GStreamer pipeline wrote a WAVE file ($sz bytes)" || bad "WAVE size: $sz" ;; *) bad "GStreamer pipeline: $(tail -n 3 gst.out)" ;; esac
else bad "gst-launch-1.0 not installed ($G)"; fi

[ $fail -eq 0 ] && { echo "PASS parole"; exit 0; }
echo "FAIL parole ($fail check(s) failed)"; exit 1
