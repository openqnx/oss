#!/bin/sh
# xfce4-appfinder: on a private broadwayd display (no screen here; GTK's
# HTML5 display server on loopback) --version and --help run, and the
# finder window starts (--disable-server, collapsed mode) with a test
# application in its menu directories and keeps running.
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
N=xfce4-appfinder
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
fhas() { awk -v p="$2" 'index($0, p) {f=1} END {exit !f}' "$1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
T=$(mktemp -d /tmp/qnxport-appfinder.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
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
HOME=$T; XDG_CONFIG_HOME=$T/config; XDG_DATA_HOME=$T/data; XDG_CACHE_HOME=$T/cache; export HOME XDG_CONFIG_HOME XDG_DATA_HOME XDG_CACHE_HOME
# --- GTK without a display: a private broadwayd (GTK's own HTML5 display
# server, here on loopback) gives GTK applications a display to open.
NO_AT_BRIDGE=1; GSETTINGS_BACKEND=memory; export NO_AT_BRIDGE GSETTINGS_BACKEND
unset DISPLAY WAYLAND_DISPLAY
if [ -z "$XDG_RUNTIME_DIR" ] || [ ! -d "$XDG_RUNTIME_DIR" ]; then XDG_RUNTIME_DIR=$T/run; mkdir -p "$XDG_RUNTIME_DIR"; chmod 700 "$XDG_RUNTIME_DIR"; export XDG_RUNTIME_DIR; fi
BWP=
bw_start() {
  [ -x "$L/bin/broadwayd" ] || { info "broadwayd (gtk3) not installed"; return 1; }
  BD=$(( $$ % 40 + 20 ))
  "$L/bin/broadwayd" --address 127.0.0.1 --port $((19000 + BD)) :$BD > "$T/broadwayd.log" 2>&1 &
  BWP=$!; track $BWP
  i=0; while [ $i -lt 15 ]; do [ -S "$XDG_RUNTIME_DIR/broadway$((BD + 1)).socket" ] && break; sleep 1; i=$((i+1)); done
  kill -0 $BWP 2>/dev/null || { info "broadwayd exited: $(head -n 3 "$T/broadwayd.log")"; return 1; }
  GDK_BACKEND=broadway; BROADWAY_DISPLAY=:$BD; export GDK_BACKEND BROADWAY_DISPLAY
}
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
# GTK lays text out with Pango; with no font installed it measures nonsense
# (VTE asked for a window 185370 px wide) and crashes. The test installs
# DejaVu (font-dejavu) and says so if fontconfig still finds nothing.
fonts_check() {
  n=$(ls "$L"/share/fonts/*/*.ttf "$L"/share/fonts/*.ttf 2>/dev/null | wc -l | tr -d ' ')
  [ "$n" -gt 0 ] && info "$n TrueType fonts under $L/share/fonts" || info "no fonts under $L/share/fonts: GTK text layout will fail"
}
# A private D-Bus session bus: Xfce programs need one (xfconf, the session
# manager), and without DBUS_SESSION_BUS_ADDRESS they try dbus-launch or
# autolaunch, neither of which exists here.
dbus_start() {
  [ -x "$L/bin/dbus-daemon" ] || { info "dbus-daemon not installed"; return 1; }
  DBUS_SESSION_BUS_ADDRESS="unix:path=$T/bus"; export DBUS_SESSION_BUS_ADDRESS
  # The system session configuration, EXTERNAL authentication: what users get.
  "$L/bin/dbus-daemon" --config-file="$L/share/dbus-1/session.conf" --address="$DBUS_SESSION_BUS_ADDRESS" --nofork --nopidfile > "$T/dbus.log" 2>&1 &
  DBP=$!; track $DBP
  i=0; while [ $i -lt 10 ]; do [ -S "$T/bus" ] && return 0; sleep 1; i=$((i+1)); done
  info "dbus-daemon did not start: $(head -n 3 "$T/dbus.log")"; return 1
}
bw_stop() { [ -n "$BWP" ] && { kill $BWP 2>/dev/null; wait $BWP 2>/dev/null; }; }
# runfor <seconds> <log> <cmd...>: 0 if the program was still running after
# <seconds> (then it is stopped) or exited 0 by itself before that.
runfor() {
  s=$1; lg=$2; shift 2
  "$@" > "$lg" 2>&1 &
  p=$!; track $p; i=0
  while [ $i -lt $s ]; do kill -0 $p 2>/dev/null || { wait $p; return $?; }; sleep 1; i=$((i+1)); done
  kill $p 2>/dev/null; wait $p 2>/dev/null; return 0
}

mkdir -p data/applications; printf '[Desktop Entry]\nType=Application\nName=QNX Test App\nExec=sh\nCategories=Utility;\n' > data/applications/qnx-test.desktop
fonts_check
dbus_start && info "private session bus at $DBUS_SESSION_BUS_ADDRESS" || bad "no D-Bus session bus (xfconf needs one)"
r=$(to 20 "$L/bin/gdbus" call --session -d org.freedesktop.DBus -o /org/freedesktop/DBus -m org.freedesktop.DBus.ListNames 2>&1)
case "$r" in *org.freedesktop.DBus*) ok "GDBus client authenticated on the session bus (EXTERNAL)" ;; *) bad "GDBus call on the session bus: $(echo "$r" | head -n 1)" ;; esac
# xfconf's daemon is started by D-Bus activation; libxfconf needs it.
r=$(to 20 "$L/bin/gdbus" call --session -d org.xfce.Xfconf -o /org/xfce/Xfconf -m org.xfce.Xfconf.ListChannels 2>&1)
case "$r" in "(["*) ok "xfconfd activated through D-Bus (ListChannels answered)" ;; *) bad "xfconfd activation: $(echo "$r" | head -n 2)"; tail -n 8 "$T/dbus.log" 2>/dev/null ;; esac
if bw_start; then
  v=$(to 30 "$L/bin/xfce4-appfinder" --version 2>/dev/null); case "$v" in *"xfce4-appfinder "[0-9]*) ok "--version: $(echo "$v" | head -n 1)" ;; *) bad "--version: $(echo "$v" | head -n 3)" ;; esac
  h=$(to 30 "$L/bin/xfce4-appfinder" --help 2>&1); case "$h" in *--collapsed*) ok "--help lists --collapsed" ;; *) bad "--help: $(echo "$h" | head -n 3)" ;; esac
  runfor 5 gui.log "$L/bin/xfce4-appfinder" --disable-server --collapsed && ok "finder window ran 5 s on broadway display $BROADWAY_DISPLAY" || { bad "finder window"; tail -n 5 gui.log; }
  runfor 4 gui2.log "$L/bin/xfrun4" --disable-server && ok "xfrun4 (run dialog) ran 4 s" || { bad "xfrun4"; tail -n 5 gui2.log; }
else bad "no broadway display: xfce4-appfinder needs one even for --version"; fi
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
