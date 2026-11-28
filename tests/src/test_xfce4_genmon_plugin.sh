#!/bin/sh
# xfce4-genmon-plugin (generic monitor for the Xfce panel): on a private
# D-Bus session bus and a headless labwc (pixman renderer). Checks: the
# module and its .desktop file are installed where the panel looks, the
# module loads (ldd) and names itself in X-XFCE-Module; the example scripts
# are installed; then the real thing: a panel configured through xfconf with
# one genmon plugin whose command is a script of the test (update period
# 1 s) is started -- the panel loads the plugin, the plugin runs the script
# again and again (the script counts its runs). Whether a command changed in
# xfconf reaches the running plugin is reported, not required. The panel is
# ended with xfce4-panel --quit.
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL xfce4_genmon_plugin (qnxport-timeout.sh missing)"; exit 1; }
echo "== xfce4-genmon-plugin =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=${QNX_PORT_PREFIX:-/usr/local}
T=$(mktemp -d /tmp/qnxport-genmon.XXXXXX) || { echo "FAIL xfce4-genmon-plugin (mktemp)"; exit 1; }
DBP=""; LWP=""; PP=""
trap '[ -n "$PP" ] && kill $PP 2>/dev/null; [ -n "$DBP" ] && kill $DBP 2>/dev/null; [ -n "$LWP" ] && kill $LWP 2>/dev/null; cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

SO=$P/lib/xfce4/panel/plugins/libgenmon.so
DT=$P/share/xfce4/panel/plugins/genmon.desktop
if [ -f "$SO" ] && [ -f "$DT" ]; then
  ok "libgenmon.so and genmon.desktop installed"
  grep -q '^X-XFCE-Module=genmon$' "$DT" && ok "genmon.desktop names the module" || bad "genmon.desktop: $(grep X-XFCE "$DT" | tr '\n' ' ')"
  ldd "$SO" > ldd.out 2>&1 && ok "libgenmon.so loads (ldd)" || bad "libgenmon.so: $(grep -i 'not found\|cannot' ldd.out | head -n 2)"
else bad "plugin not installed ($SO, $DT)"; fi
[ -f "$P/share/xfce4/genmon/scripts/datetime" ] && ok "example scripts installed" || bad "no share/xfce4/genmon/scripts"

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

# --- the monitor scripts: each run appends a line, and prints genmon markup ---
cat > mon1.sh <<EOS
#!/bin/sh
echo run >> $T/runs1
echo "<txt>QNX \$(wc -l < $T/runs1 | tr -d ' ')</txt>"
echo "<tool>generic monitor test</tool>"
EOS
cat > mon2.sh <<EOS
#!/bin/sh
echo run >> $T/runs2
echo "<txt>second</txt>"
EOS
chmod +x mon1.sh mon2.sh

# --- a panel with one genmon plugin, configured through xfconf ---
XQ="qto 30 $P/bin/xfconf-query -c xfce4-panel"
{ $XQ -p /panels -n -a -t int -s 1
  $XQ -p /panels/panel-1/position -n -t string -s "p=6;x=0;y=0"
  $XQ -p /panels/panel-1/length -n -t uint -s 100
  $XQ -p /panels/panel-1/size -n -t uint -s 28
  $XQ -p /panels/panel-1/plugin-ids -n -a -t int -s 1
  $XQ -p /plugins/plugin-1 -n -t string -s genmon
  $XQ -p /plugins/plugin-1/command -n -t string -s "$T/mon1.sh"
  $XQ -p /plugins/plugin-1/update-period -n -t int -s 1000
  $XQ -p /plugins/plugin-1/use-label -n -t bool -s false
} > xq.out 2>&1
[ "$($XQ -p /plugins/plugin-1 2>&1)" = "genmon" ] && ok "panel configuration written to xfconf" || bad "xfconf-query: $(head -n 3 xq.out)"

"$P/bin/xfce4-panel" --disable-wm-check > panel.log 2>&1 &
PP=$!
i=0; while [ $i -lt 30 ] && [ ! -s runs1 ] && kill -0 $PP 2>/dev/null; do sleep 1; i=$((i+1)); done
[ -s runs1 ] && ok "the panel loaded the plugin and it ran the command" || bad "command not run after ${i} s: $(tail -n 5 panel.log)"
n1=$(wc -l < runs1 2>/dev/null | tr -d ' '); sleep 4; n2=$(wc -l < runs1 2>/dev/null | tr -d ' ')
echo "  info runs: ${n1:-0}, 4 s later: ${n2:-0}"
[ "${n2:-0}" -ge $(( ${n1:-0} + 2 )) ] && ok "the command is run periodically (update period 1 s)" || bad "runs did not advance: ${n1:-0} -> ${n2:-0}"
kill -0 $PP 2>/dev/null && ok "xfce4-panel running" || bad "xfce4-panel exited: $(tail -n 5 panel.log)"

# --- a new command through xfconf, while running ---
$XQ -p /plugins/plugin-1/command -s "$T/mon2.sh" > /dev/null 2>&1
i=0; while [ $i -lt 15 ] && [ ! -s runs2 ]; do sleep 1; i=$((i+1)); done
if [ -s runs2 ]; then ok "a command changed in xfconf is taken over by the running plugin"
else echo "  info the running plugin did not pick up the xfconf change (it reads the command at start and from its dialog)"; fi

qto 30 "$P/bin/xfce4-panel" --quit > quit.out 2>&1
i=0; while [ $i -lt 10 ] && kill -0 $PP 2>/dev/null; do sleep 1; i=$((i+1)); done
if kill -0 $PP 2>/dev/null; then bad "xfce4-panel --quit: still running"; else PP=""; ok "xfce4-panel --quit"; fi
kill -0 $LWP 2>/dev/null || bad "labwc died: $(tail -n 3 labwc.log)"

[ $fail -eq 0 ] && { echo "PASS xfce4-genmon-plugin"; exit 0; }
echo "FAIL xfce4-genmon-plugin ($fail check(s) failed)"; exit 1
