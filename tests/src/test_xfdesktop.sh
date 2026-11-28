#!/bin/sh
# xfdesktop (Xfce desktop manager): on a private D-Bus session bus and a
# headless labwc (pixman renderer; the desktop is a wlr-layer-shell surface in
# the background layer). Checks: --version names 4.20.2; --help lists the
# remote commands; the default background image (upstream's xfce-x.svg, read
# through librsvg's gdk-pixbuf loader) is installed and is an SVG that
# rsvg-convert renders; xfdesktop starts with two files in ~/Desktop, takes
# the bus name org.xfce.xfdesktop and stays up; its settings come from xfconf
# (channel xfce4-desktop exists after start); a second invocation reaches the
# running one (--reload, --arrange, --next return 0; --arrange asks "Are you
# sure?" in a dialog unless /desktop-icons/confirm-sorting is false -- what
# "Do not ask me again" sets -- and the asking instance waits for that dialog,
# so the test sets it first); arranging the icons
# writes the positions file (YAML, ~/.config/xfce4/desktop) naming the files;
# a background colour set through xfconf is accepted while running; --quit
# ends it; xfdesktop-settings answers --help.
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL xfdesktop (qnxport-timeout.sh missing)"; exit 1; }
echo "== xfdesktop =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=${QNX_PORT_PREFIX:-/usr/local}
T=$(mktemp -d /tmp/qnxport-xfdesktop.XXXXXX) || { echo "FAIL xfdesktop (mktemp)"; exit 1; }
DBP=""; LWP=""; XP=""
trap '[ -n "$XP" ] && kill $XP 2>/dev/null; [ -n "$DBP" ] && kill $DBP 2>/dev/null; [ -n "$LWP" ] && kill $LWP 2>/dev/null; cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
X=$P/bin/xfdesktop
G="qto 30 $P/bin/gdbus"

HOME=$T/home; XDG_CONFIG_HOME=$HOME/.config; XDG_CACHE_HOME=$HOME/.cache; XDG_DATA_HOME=$HOME/.local/share
export HOME XDG_CONFIG_HOME XDG_CACHE_HOME XDG_DATA_HOME
mkdir -p "$XDG_CONFIG_HOME" "$XDG_CACHE_HOME" "$XDG_DATA_HOME" "$HOME/Desktop"
echo "hello from the desktop" > "$HOME/Desktop/qnxport-note.txt"
printf '[Desktop Entry]\nType=Application\nName=QNX port launcher\nExec=true\n' > "$HOME/Desktop/qnxport-launcher.desktop"

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

v=$(qto 60 "$X" --version 2>&1 | grep -m1 "xfdesktop")
echo "  info $v"
case "$v" in *"xfdesktop version 4.20.2"*) ok "xfdesktop --version: 4.20.2" ;; *) bad "xfdesktop --version: $v" ;; esac
h=$(qto 60 "$X" --help 2>&1)
echo "$h" | grep -q -- "--reload" && echo "$h" | grep -q -- "--arrange" && echo "$h" | grep -q -- "--quit" \
  && ok "xfdesktop --help lists --reload, --arrange and --quit" || bad "xfdesktop --help: $(echo "$h" | head -n 3)"
BG=$P/share/backgrounds/xfce/xfce-x.svg
if [ -f "$BG" ] && grep -q "<svg" "$BG"; then
  if [ -x "$P/bin/rsvg-convert" ]; then
    qto 60 "$P/bin/rsvg-convert" -w 64 "$BG" -o bg.png > bg.log 2>&1 && [ -s bg.png ] && ok "default background $BG is an SVG and renders (librsvg)" \
      || bad "default background $BG does not render: $(head -n 2 bg.log)"
  else bad "default background $BG is an SVG, but librsvg is not installed (xfdesktop depends on it)"; fi
else bad "default background: $(ls -l "$BG" 2>&1)"; fi

# --- start; bus name; stays up ---
"$X" > xfdesktop.log 2>&1 &
XP=$!
i=0; own=""
while [ $i -lt 30 ]; do
  own=$($G call --session --dest org.freedesktop.DBus --object-path /org/freedesktop/DBus --method org.freedesktop.DBus.NameHasOwner org.xfce.xfdesktop 2>&1)
  [ "$own" = "(true,)" ] && break
  kill -0 $XP 2>/dev/null || break
  sleep 1; i=$((i+1))
done
[ "$own" = "(true,)" ] && ok "xfdesktop owns org.xfce.xfdesktop on the session bus" || bad "bus name: $own; $(tail -n 4 xfdesktop.log)"
sleep 4
if kill -0 $XP 2>/dev/null; then ok "xfdesktop running on the layer-shell background surface"
else bad "xfdesktop exited: $(tail -n 5 xfdesktop.log)"; fi
grep -q "zwlr_layer_shell_v1" xfdesktop.log && bad "compositor without layer-shell: $(grep -m1 zwlr xfdesktop.log)"
kill -0 $LWP 2>/dev/null && ok "labwc still running" || bad "labwc died: $(tail -n 3 labwc.log)"

XQ="qto 30 $P/bin/xfconf-query -c xfce4-desktop"
# the channel exists once xfdesktop has written a setting of its own (its
# settings migration, shortly after the start): wait for it, at most 10 s
i=0; while [ $i -lt 10 ] && ! qto 30 "$P/bin/xfconf-query" -l 2>&1 | grep -q "xfce4-desktop"; do sleep 1; i=$((i+1)); done
qto 30 "$P/bin/xfconf-query" -l 2>&1 | grep -q "xfce4-desktop" && ok "xfconf channel xfce4-desktop" || bad "no xfconf channel xfce4-desktop: $(qto 30 "$P/bin/xfconf-query" -l 2>&1 | head -n 4)"
echo "  info properties: $($XQ -l 2>/dev/null | wc -l | tr -d ' ')"

# --- remote commands reach the running instance ---
qto 30 "$X" --reload > r.out 2>&1 && ok "xfdesktop --reload" || bad "--reload: $(head -n 2 r.out)"
qto 30 "$X" --next > r.out 2>&1 && ok "xfdesktop --next" || bad "--next: $(head -n 2 r.out)"
# no "Are you sure?" dialog: the second instance would wait for its answer
$XQ -p /desktop-icons/confirm-sorting -n -t bool -s false > xq.out 2>&1 || bad "xfconf confirm-sorting: $(head -n 2 xq.out)"
qto 30 "$X" --arrange > r.out 2>&1 && ok "xfdesktop --arrange" || bad "--arrange: $(head -n 2 r.out)"
Y=$XDG_CONFIG_HOME/xfce4/desktop/icons.screen0.yaml
i=0; while [ $i -lt 15 ] && ! grep -q "qnxport-note.txt" "$Y" 2>/dev/null; do sleep 1; i=$((i+1)); done
if grep -q "qnxport-note.txt" "$Y" 2>/dev/null && grep -q "qnxport-launcher.desktop" "$Y"; then
  ok "icon positions saved as YAML, with both files of ~/Desktop"
else bad "icon positions file: $(ls -l "$XDG_CONFIG_HOME/xfce4/desktop" 2>&1 | head -n 3) $(head -n 5 "$Y" 2>/dev/null)"; fi

# --- a setting changed through xfconf while running ---
pp=$($XQ -l 2>/dev/null | grep -m1 '/last-image$')
if [ -n "$pp" ]; then
  echo "  info $pp = $($XQ -p "$pp" 2>&1)"
  $XQ -p "$pp" -s "$BG" > xq.out 2>&1 && [ "$($XQ -p "$pp" 2>&1)" = "$BG" ] && ok "background image set through xfconf" || bad "xfconf set: $(head -n 2 xq.out)"
else
  $XQ -p /backdrop/single-workspace-mode -n -t bool -s true > xq.out 2>&1 && ok "backdrop setting written through xfconf" || bad "xfconf set: $(head -n 2 xq.out)"
fi
sleep 2
kill -0 $XP 2>/dev/null && ok "xfdesktop took the change and is still running" || bad "xfdesktop died after the change: $(tail -n 5 xfdesktop.log)"

# --- quit ---
qto 30 "$X" --quit > q.out 2>&1
i=0; while [ $i -lt 10 ] && kill -0 $XP 2>/dev/null; do sleep 1; i=$((i+1)); done
if kill -0 $XP 2>/dev/null; then bad "xfdesktop --quit: still running $(head -n 2 q.out)"; else ok "xfdesktop --quit ended it"; XP=""; fi

h=$(qto 60 "$P/bin/xfdesktop-settings" --help 2>&1)
echo "$h" | grep -q -- "--version" && ok "xfdesktop-settings --help" || bad "xfdesktop-settings --help: $(echo "$h" | head -n 3)"

[ $fail -eq 0 ] && { echo "PASS xfdesktop"; exit 0; }
echo "FAIL xfdesktop ($fail check(s) failed)"; exit 1
