#!/bin/sh
# RetroArch without a display: --version and --features, then a real run
# of a libretro core -- the 2048 game core (libretro-2048) loaded with -L,
# with null video/audio/input drivers from a private config, for a fixed
# number of frames (--max-frames) -- the verbose log must show the core
# loaded and the frame loop must end by itself.
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
N=retroarch
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
RA=$L/bin/retroarch
T=$(mktemp -d /tmp/qnxport-$N.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
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
HOME=$T; XDG_CONFIG_HOME=$T/config; export HOME XDG_CONFIG_HOME
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
v=$(to 60 "$RA" --version 2>&1); case "$v" in *RetroArch*1.*) ok "--version: $(echo "$v" | awk '/RetroArch/ {print; exit}')" ;; *) bad "--version: $(echo "$v" | head -n 3)" ;; esac
f=$(to 60 "$RA" --features 2>&1)
echo "$f" | has "Features:" && echo "$f" | has "zlib" && ok "--features: list printed (zlib among them)" || bad "--features: $(echo "$f" | head -n 3)"
echo "$f" | awk '/Threads/ && /yes/ {f=1} END {exit !f}' && ok "--features: threads supported" || info "threads: $(echo "$f" | awk '/Threads/ {print; exit}')"
CORE=$L/lib/libretro/2048_libretro_qnx.so
cat > ra.cfg <<EOF
video_driver = "null"
audio_driver = "null"
input_driver = "null"
input_joypad_driver = "null"
audio_enable = "false"
menu_driver = "null"
config_save_on_exit = "false"
libretro_directory = "$L/lib/libretro"
system_directory = "$T/system"
savefile_directory = "$T/saves"
savestate_directory = "$T/states"
log_verbosity = "true"
EOF
mkdir system saves states
if [ -f "$CORE" ]; then
  to 90 "$RA" --config "$T/ra.cfg" --verbose -L "$CORE" --max-frames=120 > run.log 2>&1; rc=$?
  has "2048" < run.log && awk '/[Cc]ore/ && /2048/ {f=1} END {exit !f}' run.log && ok "core 2048 loaded (-L $CORE)" || { bad "core not loaded"; tail -n 8 run.log; }
  [ $rc -eq 0 ] && ok "ran 120 frames with null drivers and exited (rc=0)" || { bad "run ended with rc=$rc"; awk '/[Cc]onfig|\[Video\]|driver/' run.log | head -n 12; tail -n 4 run.log; }
else bad "libretro-2048 core not installed at $CORE"; fi
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
