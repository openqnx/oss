#!/bin/sh
# oss-network-plugin ("Network" in the Xfce panel): its reading of the network
# against the system's own tools -- oss-network-info --dump (the same code as
# the plugin) must name the interface, MAC and IPv4 address/prefix that
# ifconfig shows and the gateway and interface of `route get default`, and
# call the state "connected" when there is a default route; its routing-
# socket listener must see an address being added to and removed from lo0
# (the plugin refreshes on those messages). No address is written into the
# test: whatever the target has is compared. Then the plugin in a panel on a
# private D-Bus session bus and a headless labwc: the panel loads it (memory
# map) and keeps running. The details window and "Renew DHCP Lease" need a
# pointer on QNX Screen: not covered here.
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL oss_network_plugin (qnxport-timeout.sh missing)"; exit 1; }
echo "== oss-network-plugin =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=${QNX_PORT_PREFIX:-/usr/local}
T=$(mktemp -d /tmp/qnxport-netplug.XXXXXX) || { echo "FAIL oss-network-plugin (mktemp)"; exit 1; }
DBP=""; LWP=""; PP=""
trap '[ -n "$PP" ] && kill $PP 2>/dev/null; [ -n "$DBP" ] && kill $DBP 2>/dev/null; [ -n "$LWP" ] && kill $LWP 2>/dev/null; ifconfig lo0 -alias 127.0.0.199 2>/dev/null; cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

SO=$P/lib/xfce4/panel/plugins/liboss-network.so; DT=$P/share/xfce4/panel/plugins/oss-network.desktop; NI=$P/bin/oss-network-info
if [ -f "$SO" ] && [ -f "$DT" ] && [ -x "$NI" ]; then
  ok "liboss-network.so, oss-network.desktop and oss-network-info installed"
  grep -q '^X-XFCE-Module=oss-network$' "$DT" && ok "oss-network.desktop names the module" || bad "oss-network.desktop: $(grep X-XFCE "$DT" | tr '\n' ' ')"
  ldd "$SO" > ldd.out 2>&1 && ok "liboss-network.so loads (ldd)" || bad "liboss-network.so: $(grep -i 'not found\|cannot' ldd.out | head -n 2)"
else bad "not installed ($SO, $DT, $NI)"; fi

# ---- what it reads, against ifconfig and route ----
qto 20 "$NI" --dump > dump.out 2>&1 || bad "oss-network-info --dump: $(head -n 2 dump.out)"
sed 's/^/  info /' dump.out
gw=$(qto 10 route -n get default 2>/dev/null | awk '/gateway:/ {print $2}')
gwif=$(qto 10 route -n get default 2>/dev/null | awk '/interface:/ {print $2}')
if [ -n "$gw" ]; then
  grep -q "^default $gw dev $gwif\$" dump.out && ok "default route $gw via $gwif, as route shows it" || bad "default route: route says $gw $gwif, dump: $(grep '^default' dump.out)"
  grep -q "^status connected primary=$gwif\$" dump.out && ok "status connected, on $gwif" || bad "status: $(grep '^status' dump.out)"
  ifconfig "$gwif" > ifc.out 2>&1
  a=$(awk '$1 == "inet" {print $2; exit}' ifc.out); m=$(awk '$1 == "inet" {print $4; exit}' ifc.out)
  bits=0; case "$m" in 0x*) h=${m#0x}; while [ -n "$h" ]; do d=$(printf '%d' "0x$(echo "$h" | cut -c1)"); while [ "$d" -gt 0 ]; do bits=$((bits + (d & 1))); d=$((d >> 1)); done; h=$(echo "$h" | cut -c2-); done ;; esac
  grep -q "^  inet $a/$bits\$" dump.out && ok "IPv4 $a/$bits on $gwif, as ifconfig shows it" || bad "IPv4: ifconfig $a/$bits ($m), dump: $(grep '  inet ' dump.out | head -n 2 | tr '\n' ' ')"
  mac=$(awk '$1 == "ether" {print $2; exit}' ifc.out)
  [ -z "$mac" ] || { grep -q "^interface $gwif .*mac=$mac" dump.out && ok "MAC $mac, as ifconfig shows it" || bad "MAC: ifconfig $mac, dump: $(grep "^interface $gwif" dump.out)"; }
  grep -q "^interface $gwif up=1 running=1 link=up" dump.out && ok "$gwif up, running, link up" || bad "link state: $(grep "^interface $gwif" dump.out)"
else
  echo "  info no default route here: $(grep '^status' dump.out)"
  grep -q '^default none$' dump.out && ok "no default route, and it says so" || bad "route has no default, dump: $(grep '^default' dump.out)"
fi

# ---- the routing-socket listener sees changes ----
qto 20 "$NI" --events 4 > ev.out 2>&1 &
EP=$!; sleep 1
ifconfig lo0 alias 127.0.0.199/32 2>/dev/null; sleep 1; ifconfig lo0 -alias 127.0.0.199 2>/dev/null
wait $EP
n=$(awk '/^events / {print $2}' ev.out)
[ "${n:-0}" -ge 1 ] && ok "routing messages seen for an address added to and removed from lo0 ($n)" || bad "no routing messages: $(tail -n 2 ev.out)"

# ---- in a panel ----
HOME=$T/home; XDG_CONFIG_HOME=$HOME/.config; XDG_CACHE_HOME=$HOME/.cache; XDG_DATA_HOME=$HOME/.local/share
export HOME XDG_CONFIG_HOME XDG_CACHE_HOME XDG_DATA_HOME
mkdir -p "$XDG_CONFIG_HOME" "$XDG_CACHE_HOME" "$XDG_DATA_HOME"
RT=$T/rt; mkdir -m 700 "$RT"
XDG_RUNTIME_DIR=$RT WLR_BACKENDS=headless WLR_RENDERER=pixman "$P/bin/labwc" > labwc.log 2>&1 &
LWP=$!
i=0; while [ ! -S "$RT/wayland-0" ] && [ $i -lt 150 ]; do sleep 0.1; i=$((i+1)); done
XDG_RUNTIME_DIR=$RT; WAYLAND_DISPLAY=wayland-0; GDK_BACKEND=wayland; export XDG_RUNTIME_DIR WAYLAND_DISPLAY GDK_BACKEND
DBUS_SESSION_BUS_ADDRESS="unix:path=$T/bus"; export DBUS_SESSION_BUS_ADDRESS
"$P/bin/dbus-daemon" --config-file="$P/share/dbus-1/session.conf" --address="$DBUS_SESSION_BUS_ADDRESS" --nofork --nopidfile > dbus.log 2>&1 &
DBP=$!
i=0; while [ $i -lt 10 ] && [ ! -S "$T/bus" ]; do sleep 1; i=$((i+1)); done
XQ="qto 30 $P/bin/xfconf-query -c xfce4-panel"
{ $XQ -p /panels -n -a -t int -s 1
  $XQ -p /panels/panel-1/position -n -t string -s "p=6;x=0;y=0"
  $XQ -p /panels/panel-1/size -n -t uint -s 28
  $XQ -p /panels/panel-1/plugin-ids -n -a -t int -s 1
  $XQ -p /plugins/plugin-1 -n -t string -s oss-network
} > xq.out 2>&1
"$P/bin/xfce4-panel" --disable-wm-check > panel.log 2>&1 &
PP=$!
sleep 6
if kill -0 $PP 2>/dev/null; then
  ok "xfce4-panel with the plugin running after 6 s"
  # the panel loads its plugins after it is up: up to 30 s more (slow emulated targets)
  i=0; while :; do pidin -p $PP mem > mem.out 2>&1; grep -q 'liboss-network\.so' mem.out && break
    [ $i -ge 30 ] || ! kill -0 $PP 2>/dev/null && break; sleep 1; i=$((i+1)); done
  grep -q 'liboss-network\.so' mem.out && ok "the panel loaded the plugin (liboss-network.so mapped)" || bad "liboss-network.so not in the panel's memory map: $(tail -n 3 panel.log)"
else bad "xfce4-panel exited: $(tail -n 5 panel.log)"; fi
qto 30 "$P/bin/xfce4-panel" --quit > quit.out 2>&1
i=0; while [ $i -lt 10 ] && kill -0 $PP 2>/dev/null; do sleep 1; i=$((i+1)); done
kill -0 $PP 2>/dev/null && bad "xfce4-panel --quit: still running" || { PP=""; ok "xfce4-panel --quit"; }

[ $fail -eq 0 ] && { echo "PASS oss-network-plugin"; exit 0; }
echo "FAIL oss-network-plugin ($fail check(s) failed)"; exit 1
