#!/bin/sh
# librsvg: rsvg-convert renders an SVG made here (shapes, a gradient and text
# through Pango) to PNG at a given width -- the PNG must have that size --
# and an icon of the installed icon themes; the gdk-pixbuf SVG loader must be
# known to gdk-pixbuf-query-loaders (what makes every GTK program read SVG:
# symbolic icons, xfdesktop's SVG backgrounds); a broken SVG must be refused
# with an error, not a crash.
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
N=librsvg
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
fhas() { awk -v p="$2" 'index($0, p) {f=1} END {exit !f}' "$1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
T=$(mktemp -d /tmp/qnxport-rsvg.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
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
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL librsvg (qnxport-timeout.sh missing)"; exit 1; }
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

R=$L/bin/rsvg-convert
v=$(to 20 "$R" --version 2>&1 | head -n 1)
case "$v" in *2.60*) ok "rsvg-convert --version: $v" ;; *) bad "--version: '$v'" ;; esac
cat > t.svg <<'SVG'
<svg xmlns="http://www.w3.org/2000/svg" width="100" height="50" viewBox="0 0 100 50">
  <defs><linearGradient id="g"><stop offset="0" stop-color="#1a5fb4"/><stop offset="1" stop-color="#26a269"/></linearGradient></defs>
  <rect width="100" height="50" rx="8" fill="url(#g)"/>
  <circle cx="20" cy="25" r="12" fill="#f6d32d"/>
  <text x="62" y="32" font-size="18" text-anchor="middle" fill="white">QNX</text>
</svg>
SVG
to 30 "$R" -w 300 t.svg -o t.png > conv.log 2>&1; r=$?
# PNG: signature, then the IHDR width and height (big endian) at bytes 16..23
sig=$(od -An -tx1 -N8 t.png 2>/dev/null | tr -d ' \n')
wh=$(od -An -tu1 -j16 -N8 t.png 2>/dev/null | awk '{print $3*256+$4 "x" $7*256+$8}')
[ $r -eq 0 ] && [ "$sig" = 89504e470d0a1a0a ] && [ "$wh" = 300x150 ] && ok "rsvg-convert -w 300: PNG $wh" || bad "rsvg-convert: rc=$r sig=$sig size=$wh $(head -n 2 conv.log)"
ico=$(ls "$L"/share/icons/Adwaita/symbolic/*/*.svg "$L"/share/icons/*/scalable/*/*.svg 2>/dev/null | head -n 1)
if [ -n "$ico" ]; then
  to 30 "$R" -w 64 "$ico" -o ico.png > ico.log 2>&1 && [ -s ico.png ] && ok "icon $(basename "$ico") rendered" || bad "icon $ico: $(head -n 2 ico.log)"
else info "no SVG icon theme installed: icon not rendered"; fi
Q=$(command -v gdk-pixbuf-query-loaders || echo "$L/bin/gdk-pixbuf-query-loaders")
if [ -x "$Q" ]; then
  "$Q" 2>/dev/null | grep -q 'pixbufloader_svg' && "$Q" 2>/dev/null | grep -q '"image/svg+xml"' && ok "gdk-pixbuf knows the SVG loader (image/svg+xml)" || bad "gdk-pixbuf-query-loaders does not list the SVG loader"
else info "gdk-pixbuf-query-loaders not installed"; fi
printf '<svg xmlns="http://www.w3.org/2000/svg"><rect width=' > bad.svg
to 30 "$R" bad.svg -o bad.png > bad.log 2>&1; r=$?
case $r in 0|124|13[0-9]|14[0-9]) bad "broken SVG: exit $r" ;; *) ok "a broken SVG is refused (exit $r)" ;; esac
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
