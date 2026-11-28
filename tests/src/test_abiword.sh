#!/bin/sh
# AbiWord: --version, then a real Word 97 document (AbiWord's own test file
# test/wp/Word97Test.doc, GPL, embedded below gzip'd and base64'd) converted
# from the command line (abiword --to=...): to text -- the .doc import (wv)
# -- and to OpenDocument and Office Open XML, which are then converted back to
# text -- the opendocument and openxml plugins both ways. The sentences of the
# document must come through every step. AbiWord initialises GTK even to
# convert, so it runs on a private broadwayd display; then the window itself
# for a few seconds with the document open.
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
N=abiword
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
fhas() { awk -v p="$2" 'index($0, p) {f=1} END {exit !f}' "$1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
T=$(mktemp -d /tmp/qnxport-abiword.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
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
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL abiword (qnxport-timeout.sh missing)"; exit 1; }
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

cat > doc.b64 <<'B64'
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
TQe1AFIAAA==
B64
base64 -d doc.b64 2>/dev/null | gzip -dc > w97.doc 2>/dev/null
[ "$(wc -c < w97.doc | tr -d ' ')" = 20992 ] && ok "test document: 20992 bytes" || bad "test document not decoded"
A=$L/bin/abiword
fonts_check
dbus_start || :
if bw_start; then
  v=$(to 30 "$A" --version 2>&1 | grep -E '^[0-9]+\.[0-9]' | head -n 1)
  [ "$v" = 3.0.8 ] && ok "--version: $v" || bad "--version: '$v'"
  has() { hf=$1; shift; for s in "$@"; do fhas "$hf" "$s" || return 1; done; }
  S1="Times New Roman 10pt in bold"; S2="Verdana (Baltic) at 18 point"
  to 60 "$A" --to=txt -o w97.txt w97.doc > c1.log 2>&1
  has w97.txt "$S1" "$S2" && ok ".doc -> text (wv import)" || bad ".doc -> text: $(head -n 2 c1.log)"
  for f in odt docx; do
    to 60 "$A" --to=$f -o w97.$f w97.doc > c2-$f.log 2>&1
    if [ -s w97.$f ] && [ "$(head -c 2 w97.$f)" = PK ]; then
      to 60 "$A" --to=txt -o back-$f.txt w97.$f > c3-$f.log 2>&1
      has back-$f.txt "$S1" "$S2" && ok ".doc -> .$f -> text (the $f plugin both ways)" || bad ".$f -> text: $(head -n 2 c3-$f.log)"
    else bad ".doc -> .$f: $(head -n 2 c2-$f.log)"; fi
  done
  runfor 6 gui.log "$A" w97.doc && ok "window with the document ran 6 s on broadway display $BROADWAY_DISPLAY" || { bad "abiword window"; tail -n 5 gui.log; }
else bad "no broadway display"; fi
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
