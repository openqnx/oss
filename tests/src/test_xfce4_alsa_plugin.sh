#!/bin/sh
# xfce4-alsa-plugin (ALSA volume control for the Xfce panel, on QNX's
# libasound): on a private D-Bus session bus and a headless labwc (pixman
# renderer). Checks: the module and its .desktop file are installed where
# the panel looks, the module loads (ldd, libasound.so.5 among its
# libraries) and names itself in X-XFCE-Module; whether a sound card is
# there (/dev/snd) is reported -- the test does not need one; then a panel
# configured through xfconf with the plugin alone is started: it loads the
# module (seen in the panel's memory map) and keeps running for 6 s, with a
# card or without (then the button says "No sound card found",
# patches/0002); xfce4-panel --quit ends it.
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL xfce4_alsa_plugin (qnxport-timeout.sh missing)"; exit 1; }
echo "== xfce4-alsa-plugin =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=${QNX_PORT_PREFIX:-/usr/local}
T=$(mktemp -d /tmp/qnxport-alsaplug.XXXXXX) || { echo "FAIL xfce4-alsa-plugin (mktemp)"; exit 1; }
DBP=""; LWP=""; PP=""
trap '[ -n "$PP" ] && kill $PP 2>/dev/null; [ -n "$DBP" ] && kill $DBP 2>/dev/null; [ -n "$LWP" ] && kill $LWP 2>/dev/null; cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

SO=$P/lib/xfce4/panel/plugins/libalsa.so
DT=$P/share/xfce4/panel/plugins/alsa.desktop
if [ -f "$SO" ] && [ -f "$DT" ]; then
  ok "libalsa.so and alsa.desktop installed"
  grep -q '^X-XFCE-Module=alsa$' "$DT" && ok "alsa.desktop names the module" || bad "alsa.desktop: $(grep X-XFCE "$DT" | tr '\n' ' ')"
  if ldd "$SO" > ldd.out 2>&1; then
    grep -q 'libasound\.so' ldd.out && ok "libalsa.so loads (ldd), with libasound.so.5" || bad "libalsa.so: no libasound in ldd output"
  else bad "libalsa.so: $(grep -i 'not found\|cannot' ldd.out | head -n 2)"; fi
  grep -q 'No sound card found' "$SO" && ok "the no-card message is built in (patches/0002)" || bad "no \"No sound card found\" in libalsa.so"
else bad "plugin not installed ($SO, $DT)"; fi
if [ -d /dev/snd ]; then echo "  info a sound card is there: $(ls /dev/snd | tr '\n' ' ')"
else echo "  info no sound card (/dev/snd absent; io-snd not running): the no-card path"; fi

HOME=$T/home; XDG_CONFIG_HOME=$HOME/.config; XDG_CACHE_HOME=$HOME/.cache; XDG_DATA_HOME=$HOME/.local/share
export HOME XDG_CONFIG_HOME XDG_CACHE_HOME XDG_DATA_HOME
mkdir -p "$XDG_CONFIG_HOME" "$XDG_CACHE_HOME" "$XDG_DATA_HOME"
RT=$T/rt; mkdir -m 700 "$RT"
XDG_RUNTIME_DIR=$RT WLR_BACKENDS=headless WLR_RENDERER=pixman "$P/bin/labwc" > labwc.log 2>&1 &
LWP=$!
i=0; while [ ! -S "$RT/wayland-0" ] && [ $i -lt 150 ]; do sleep 0.1; i=$((i+1)); done
[ -S "$RT/wayland-0" ] && ok "labwc up (headless)" || bad "labwc: $(tail -n 2 labwc.log)"
XDG_RUNTIME_DIR=$RT; WAYLAND_DISPLAY=wayland-0; GDK_BACKEND=wayland; export XDG_RUNTIME_DIR WAYLAND_DISPLAY GDK_BACKEND
if [ -x "$P/bin/dbus-daemon" ]; then
  DBUS_SESSION_BUS_ADDRESS="unix:path=$T/bus"; export DBUS_SESSION_BUS_ADDRESS
  "$P/bin/dbus-daemon" --config-file="$P/share/dbus-1/session.conf" --address="$DBUS_SESSION_BUS_ADDRESS" --nofork --nopidfile > dbus.log 2>&1 &
  DBP=$!
  i=0; while [ $i -lt 10 ] && [ ! -S "$T/bus" ]; do sleep 1; i=$((i+1)); done
  [ -S "$T/bus" ] && ok "private D-Bus session bus" || bad "dbus-daemon: $(head -n 2 dbus.log)"
else bad "dbus-daemon not installed"; fi

XQ="qto 30 $P/bin/xfconf-query -c xfce4-panel"
{ $XQ -p /panels -n -a -t int -s 1
  $XQ -p /panels/panel-1/position -n -t string -s "p=6;x=0;y=0"
  $XQ -p /panels/panel-1/length -n -t uint -s 100
  $XQ -p /panels/panel-1/size -n -t uint -s 28
  $XQ -p /panels/panel-1/plugin-ids -n -a -t int -s 1
  $XQ -p /plugins/plugin-1 -n -t string -s alsa
} > xq.out 2>&1
[ "$($XQ -p /plugins/plugin-1 2>&1)" = "alsa" ] && ok "panel configuration written to xfconf" || bad "xfconf-query: $(head -n 3 xq.out)"

"$P/bin/xfce4-panel" --disable-wm-check > panel.log 2>&1 &
PP=$!
sleep 6
if kill -0 $PP 2>/dev/null; then
  ok "xfce4-panel with the plugin running after 6 s"
  # the panel loads its plugins after it is up: up to 30 s more (slow emulated targets)
  i=0; while :; do pidin -p $PP mem > mem.out 2>&1; grep -q 'libalsa\.so' mem.out && break
    [ $i -ge 30 ] || ! kill -0 $PP 2>/dev/null && break; sleep 1; i=$((i+1)); done
  grep -q 'libalsa\.so' mem.out && ok "the panel loaded the plugin (libalsa.so mapped)" || bad "libalsa.so not in the panel's memory map: $(grep -i -E 'alsa|plugin' panel.log | head -n 3)"
else bad "xfce4-panel exited: $(tail -n 5 panel.log)"; fi
grep -i -E 'alsa|critical' panel.log | head -n 3 | sed 's/^/  info /'

qto 30 "$P/bin/xfce4-panel" --quit > quit.out 2>&1
i=0; while [ $i -lt 10 ] && kill -0 $PP 2>/dev/null; do sleep 1; i=$((i+1)); done
if kill -0 $PP 2>/dev/null; then bad "xfce4-panel --quit: still running"; else PP=""; ok "xfce4-panel --quit"; fi
kill -0 $LWP 2>/dev/null || bad "labwc died: $(tail -n 3 labwc.log)"

[ $fail -eq 0 ] && { echo "PASS xfce4-alsa-plugin"; exit 0; }
echo "FAIL xfce4-alsa-plugin ($fail check(s) failed)"; exit 1
