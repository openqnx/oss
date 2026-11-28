#!/bin/sh
# xfce4-cpugraph-plugin in a running panel (test_xfce4_cpugraph_plugin.c
# checks the CPU-time clocks the plugin reads): on a private
# D-Bus session bus and a headless labwc (pixman renderer). Checks: the
# module and its .desktop file are installed where the panel looks, the
# module loads (ldd) and names itself in X-XFCE-Module; what the QNX backend
# (patches/0001) relies on holds on this target -- the kernel process has one
# idle thread per processor, thread ids 1..num_cpu at priority 0; then a
# panel configured through xfconf with one cpugraph plugin is started: the
# panel loads the plugin, the plugin sets up its processors (no "Cannot init
# cpu data") and stays up for 5 s of 250 ms updates; on xfce4-panel --quit
# it saves its settings to xfconf, which shows it was loaded and running.
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL xfce4_cpugraph_plugin (qnxport-timeout.sh missing)"; exit 1; }
echo "== xfce4-cpugraph-plugin =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=${QNX_PORT_PREFIX:-/usr/local}
T=$(mktemp -d /tmp/qnxport-cpugraph.XXXXXX) || { echo "FAIL xfce4-cpugraph-plugin (mktemp)"; exit 1; }
DBP=""; LWP=""; PP=""
trap '[ -n "$PP" ] && kill $PP 2>/dev/null; [ -n "$DBP" ] && kill $DBP 2>/dev/null; [ -n "$LWP" ] && kill $LWP 2>/dev/null; cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

SO=$P/lib/xfce4/panel/plugins/libcpugraph.so
DT=$P/share/xfce4/panel/plugins/cpugraph.desktop
if [ -f "$SO" ] && [ -f "$DT" ]; then
  ok "libcpugraph.so and cpugraph.desktop installed"
  grep -q '^X-XFCE-Module=cpugraph$' "$DT" && ok "cpugraph.desktop names the module" || bad "cpugraph.desktop: $(grep X-XFCE "$DT" | tr '\n' ' ')"
  ldd "$SO" > ldd.out 2>&1 && ok "libcpugraph.so loads (ldd)" || bad "libcpugraph.so: $(grep -i 'not found\|cannot' ldd.out | head -n 2)"
else bad "plugin not installed ($SO, $DT)"; fi

# --- what the backend reads: procnto's idle threads, ids 1..num_cpu ---
ncpu=$(pidin info 2>/dev/null | grep -c '^Processor[0-9]')
pidin -p 1 -F "%a %b %p" > idle.out 2>&1
nidle=0; i=1
while [ $i -le "${ncpu:-0}" ]; do
  awk -v t=$i '$1 == 1 && $2 == t && $3 ~ /^0[a-z]?$/ { f = 1 } END { exit !f }' idle.out && nidle=$((nidle+1))
  i=$((i+1))
done
echo "  info $ncpu processor(s), $nidle idle thread(s) at ids 1..$ncpu"
[ "${ncpu:-0}" -ge 1 ] && [ "$nidle" -eq "$ncpu" ] && ok "one idle thread per processor in process 1 (ids 1..$ncpu, priority 0)" \
  || bad "idle threads of process 1 not as expected: $(head -n 6 idle.out | tr '\n' ' ')"

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

# --- a panel with one cpugraph plugin, configured through xfconf ---
XQ="qto 30 $P/bin/xfconf-query -c xfce4-panel"
{ $XQ -p /panels -n -a -t int -s 1
  $XQ -p /panels/panel-1/position -n -t string -s "p=6;x=0;y=0"
  $XQ -p /panels/panel-1/length -n -t uint -s 100
  $XQ -p /panels/panel-1/size -n -t uint -s 28
  $XQ -p /panels/panel-1/plugin-ids -n -a -t int -s 1
  $XQ -p /plugins/plugin-1 -n -t string -s cpugraph
  $XQ -p /plugins/plugin-1/update-interval -n -t int -s 0
} > xq.out 2>&1
[ "$($XQ -p /plugins/plugin-1 2>&1)" = "cpugraph" ] && ok "panel configuration written to xfconf" || bad "xfconf-query: $(head -n 3 xq.out)"

"$P/bin/xfce4-panel" --disable-wm-check > panel.log 2>&1 &
PP=$!
sleep 6
if kill -0 $PP 2>/dev/null; then ok "xfce4-panel with the plugin running after 6 s"; else bad "xfce4-panel exited: $(tail -n 5 panel.log)"; fi
grep -q 'Cannot init cpu data' panel.log && bad "the plugin found no processors: $(grep -i cpu panel.log | head -n 2)" || ok "the plugin set up its processors (no \"Cannot init cpu data\")"
grep -i -E 'cpugraph.*(fail|error|crit)|libcpugraph' panel.log | head -n 3 | sed 's/^/  info /'

qto 30 "$P/bin/xfce4-panel" --quit > quit.out 2>&1
i=0; while [ $i -lt 10 ] && kill -0 $PP 2>/dev/null; do sleep 1; i=$((i+1)); done
if kill -0 $PP 2>/dev/null; then bad "xfce4-panel --quit: still running"; else PP=""; ok "xfce4-panel --quit"; fi
$XQ -p /plugins/plugin-1 -l > props.out 2>&1
np=$(grep -c '^/plugins/plugin-1/' props.out)
echo "  info plugin properties after quit: $np"
grep -q '^/plugins/plugin-1/size$' props.out && ok "the plugin saved its settings to xfconf (it was loaded and running)" || bad "no settings saved by the plugin: $(tr '\n' ' ' < props.out | head -c 200)"
kill -0 $LWP 2>/dev/null || bad "labwc died: $(tail -n 3 labwc.log)"

[ $fail -eq 0 ] && { echo "PASS xfce4-cpugraph-plugin"; exit 0; }
echo "FAIL xfce4-cpugraph-plugin ($fail check(s) failed)"; exit 1
