#!/bin/sh
# xfce4-notifyd (Xfce notification daemon): on a private D-Bus session bus and
# a headless labwc (pixman renderer; the pop-ups are wlr-layer-shell surfaces).
# Checks: the daemon prints its version; the session bus starts it by D-Bus
# activation on the first call to org.freedesktop.Notifications;
# GetServerInformation names "Xfce Notify Daemon" 0.9.7 and GetCapabilities
# lists body and actions; notify-send (libnotify) delivers a notification and
# gets an id; a second Notify with replaces_id keeps the id; CloseNotification
# is answered with the NotificationClosed signal (reason 3); with logging set
# to "always" through xfconf the notification is in the SQLite log and
# org.xfce.Notifyd.Log.List returns it; "do not disturb" is taken from xfconf
# while running; the settings program and the panel plugin load;
# org.xfce.Notifyd.Quit ends the daemon.
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL xfce4_notifyd (qnxport-timeout.sh missing)"; exit 1; }
echo "== xfce4-notifyd =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=${QNX_PORT_PREFIX:-/usr/local}
T=$(mktemp -d /tmp/qnxport-notifyd.XXXXXX) || { echo "FAIL xfce4-notifyd (mktemp)"; exit 1; }
DBP=""; LWP=""; MP=""
trap '[ -n "$MP" ] && kill $MP 2>/dev/null; [ -n "$DBP" ] && kill $DBP 2>/dev/null; [ -n "$LWP" ] && kill $LWP 2>/dev/null; cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
ND=$P/lib/xfce4/notifyd/xfce4-notifyd
G="$P/bin/gdbus"   # every call has --timeout: QNX's timeout(1) was seen waiting on after its child had ended
FDO="--session --dest org.freedesktop.Notifications --object-path /org/freedesktop/Notifications"

HOME=$T/home; XDG_CONFIG_HOME=$HOME/.config; XDG_CACHE_HOME=$HOME/.cache; XDG_DATA_HOME=$HOME/.local/share
export HOME XDG_CONFIG_HOME XDG_CACHE_HOME XDG_DATA_HOME
mkdir -p "$XDG_CONFIG_HOME" "$XDG_CACHE_HOME" "$XDG_DATA_HOME"
RT=$T/rt; mkdir -m 700 "$RT"
XDG_RUNTIME_DIR=$RT WLR_BACKENDS=headless WLR_RENDERER=pixman "$P/bin/labwc" > labwc.log 2>&1 &
LWP=$!
i=0; while [ ! -S "$RT/wayland-0" ] && [ $i -lt 150 ]; do sleep 0.1; i=$((i+1)); done
[ -S "$RT/wayland-0" ] && ok "labwc up (headless)" || bad "labwc: $(tail -n 2 labwc.log)"
XDG_RUNTIME_DIR=$RT; WAYLAND_DISPLAY=wayland-0; GDK_BACKEND=wayland; export XDG_RUNTIME_DIR WAYLAND_DISPLAY GDK_BACKEND

v=$(qto 30 "$ND" --version 2>&1 | head -n 1)
case "$v" in *"0.9.7"*) ok "xfce4-notifyd --version: $v" ;; *) bad "xfce4-notifyd --version: $v" ;; esac
for f in org.xfce.xfce4-notifyd.Notifications.service org.xfce.xfce4-notifyd.Notifyd.service; do
  grep -q "^Exec=$ND" "$P/share/dbus-1/services/$f" 2>/dev/null || bad "D-Bus activation file $f"
done
[ -f "$P/etc/xdg/autostart/xfce4-notifyd.desktop" ] && ok "D-Bus activation files and XDG autostart entry" || bad "no etc/xdg/autostart/xfce4-notifyd.desktop"

# The bus is started after the Wayland variables are exported: the daemon it
# activates inherits them.
if [ -x "$P/bin/dbus-daemon" ]; then
  DBUS_SESSION_BUS_ADDRESS="unix:path=$T/bus"; export DBUS_SESSION_BUS_ADDRESS
  "$P/bin/dbus-daemon" --config-file="$P/share/dbus-1/session.conf" --address="$DBUS_SESSION_BUS_ADDRESS" --nofork --nopidfile > dbus.log 2>&1 &
  DBP=$!
  i=0; while [ $i -lt 10 ] && [ ! -S "$T/bus" ]; do sleep 1; i=$((i+1)); done
  [ -S "$T/bus" ] && ok "private D-Bus session bus" || bad "dbus-daemon: $(head -n 2 dbus.log)"
else bad "dbus-daemon not installed"; fi

# Log every notification (the default logs only those not fully shown).
XQ="qto 30 $P/bin/xfconf-query -c xfce4-notifyd"
$XQ -p /notification-log -n -t bool -s true > xq.out 2>&1
$XQ -p /log-level -n -t string -s always >> xq.out 2>&1
[ "$($XQ -p /log-level 2>&1)" = "always" ] && ok "xfconf: log-level always" || bad "xfconf-query: $(head -n 3 xq.out)"

# --- D-Bus activation, server information, capabilities ---
si=$($G call --timeout 10 $FDO --method org.freedesktop.Notifications.GetServerInformation 2>&1)
echo "  info $si"
case "$si" in *"'Xfce Notify Daemon', 'Xfce', '0.9.7'"*) ok "started by the bus; GetServerInformation: Xfce Notify Daemon 0.9.7" ;; *) bad "GetServerInformation: $si $(tail -n 3 dbus.log)" ;; esac
cp=$($G call --timeout 10 $FDO --method org.freedesktop.Notifications.GetCapabilities 2>&1)
case "$cp" in *"'actions'"*"'body'"*) ok "GetCapabilities lists actions and body" ;; *) bad "GetCapabilities: $cp" ;; esac

"$P/bin/gdbus" monitor $FDO > mon.out 2>&1 &
MP=$!
sleep 1

# --- notifications ---
id1=$(qto 30 "$P/bin/notify-send" -p -a qnxport-test -t 20000 "QNX port test" "first body" 2> ns.err)
case "$id1" in ''|*[!0-9]*) bad "notify-send -p: '$id1' $(head -n 2 ns.err)" ;; *) ok "notify-send delivered a notification (id $id1)" ;; esac
r=$($G call --timeout 10 $FDO --method org.freedesktop.Notifications.Notify qnxport-test 0 "" "second summary" "second body" "['default', 'Open']" "{'urgency': <byte 1>}" 20000 2>&1)
id2=$(echo "$r" | sed -n 's/^(uint32 \([0-9]*\),)$/\1/p')
[ -n "$id2" ] && [ "$id2" != "$id1" ] && ok "Notify with an action and a hint: id $id2" || bad "Notify: $r"
r=$($G call --timeout 10 $FDO --method org.freedesktop.Notifications.Notify qnxport-test "${id2:-0}" "" "replaced summary" "replaced body" "[]" "{}" 20000 2>&1)
id3=$(echo "$r" | sed -n 's/^(uint32 \([0-9]*\),)$/\1/p')
[ -n "$id3" ] && [ "$id3" = "$id2" ] && ok "replaces_id keeps the id" || bad "replace: $r (was $id2)"
sleep 1
$G call --timeout 10 $FDO --method org.freedesktop.Notifications.CloseNotification "${id2:-0}" > close.out 2>&1
i=0; while [ $i -lt 10 ] && ! grep -q "NotificationClosed" mon.out 2>/dev/null; do sleep 1; i=$((i+1)); done
grep "NotificationClosed" mon.out | grep -q "(uint32 ${id2:-x}, uint32 3)" && ok "CloseNotification -> NotificationClosed (reason 3)" || bad "NotificationClosed: $(grep -m2 Closed mon.out) $(head -n 2 close.out)"
kill $MP 2>/dev/null; MP=""
kill -0 $LWP 2>/dev/null && ok "labwc still running with the pop-ups (layer-shell surfaces)" || bad "labwc died: $(tail -n 3 labwc.log)"

# --- the log ---
LOGDB=$XDG_CACHE_HOME/xfce4/notifyd/log.sqlite
i=0; while [ $i -lt 10 ] && [ ! -s "$LOGDB" ]; do sleep 1; i=$((i+1)); done
[ "$(head -c 15 "$LOGDB" 2>/dev/null)" = "SQLite format 3" ] && ok "log database $LOGDB" || bad "no SQLite log: $(ls -l "$XDG_CACHE_HOME/xfce4/notifyd" 2>&1 | head -n 3)"
ll=$($G call --timeout 10 --session --dest org.xfce.Notifyd --object-path /org/xfce/Notifyd --method org.xfce.Notifyd.Log.List "" 10 false 2>&1)
case "$ll" in *"QNX port test"*"first body"*|*"first body"*"QNX port test"*) ok "org.xfce.Notifyd.Log.List returns the logged notification" ;; *) bad "Log.List: $(echo "$ll" | head -c 300)" ;; esac
hu=$($G call --timeout 10 --session --dest org.xfce.Notifyd --object-path /org/xfce/Notifyd --method org.xfce.Notifyd.Log.HasUnread 2>&1)
[ "$hu" = "(true,)" ] && ok "Log.HasUnread: true" || bad "HasUnread: $hu"
$G call --timeout 10 --session --dest org.xfce.Notifyd --object-path /org/xfce/Notifyd --method org.xfce.Notifyd.Log.MarkAllRead > /dev/null 2>&1
hu=$($G call --timeout 10 --session --dest org.xfce.Notifyd --object-path /org/xfce/Notifyd --method org.xfce.Notifyd.Log.HasUnread 2>&1)
[ "$hu" = "(false,)" ] && ok "Log.MarkAllRead" || bad "after MarkAllRead: $hu"

# --- do not disturb: taken from xfconf while running; still logged ---
$XQ -p /do-not-disturb -n -t bool -s true > /dev/null 2>&1
sleep 1
qto 30 "$P/bin/notify-send" -a qnxport-test "quiet one" "not shown" > /dev/null 2>&1
sleep 1
ll=$($G call --timeout 10 --session --dest org.xfce.Notifyd --object-path /org/xfce/Notifyd --method org.xfce.Notifyd.Log.List "" 10 true 2>&1)
case "$ll" in *"quiet one"*) ok "do-not-disturb: the notification went to the log, unread" ;; *) bad "do-not-disturb log: $(echo "$ll" | head -c 200)" ;; esac

# --- settings program and panel plugin ---
h=$(qto 30 "$P/bin/xfce4-notifyd-config" --help 2>&1)
echo "$h" | grep -q -- "--version" && ok "xfce4-notifyd-config --help" || bad "xfce4-notifyd-config --help: $(echo "$h" | head -n 3)"
PL=$P/lib/xfce4/panel/plugins/libnotification-plugin.so
if [ -f "$PL" ] && [ -f "$P/share/xfce4/panel/plugins/notification-plugin.desktop" ]; then
  ldd "$PL" > ldd.out 2>&1 && ok "panel plugin libnotification-plugin.so loads (ldd)" || bad "panel plugin: $(grep -i 'not found\|cannot' ldd.out | head -n 2)"
else bad "panel plugin not installed"; fi

# --- quit ---
$G call --timeout 10 --session --dest org.xfce.Notifyd --object-path /org/xfce/Notifyd --method org.xfce.Notifyd.Quit > /dev/null 2>&1
sleep 2
o=$($G call --timeout 10 --session --dest org.freedesktop.DBus --object-path /org/freedesktop/DBus --method org.freedesktop.DBus.NameHasOwner org.freedesktop.Notifications 2>&1)
[ "$o" = "(false,)" ] && ok "org.xfce.Notifyd.Quit ended the daemon" || bad "after Quit: NameHasOwner $o"

[ $fail -eq 0 ] && { echo "PASS xfce4-notifyd"; exit 0; }
echo "FAIL xfce4-notifyd ($fail check(s) failed)"; exit 1
