#!/bin/sh
# EmulationStation without a display: --help prints its options and
# version; then a real start with a private home (--home) and a test
# system in es_systems.cfg -- ES writes its log and settings and reads the
# systems file, and must stop cleanly with an error at the video
# initialisation, since there is no screen to open.
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
N=emulationstation
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
ES=$L/bin/emulationstation
T=$(mktemp -d /tmp/qnxport-es.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
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
HOME=$T; export HOME; unset DISPLAY WAYLAND_DISPLAY
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
h=$(to 60 "$ES" --help 2>&1)
echo "$h" | has "EmulationStation, a graphical front-end" && echo "$h" | awk '/^Version [0-9]/ {f=1} END {exit !f}' && ok "--help: $(echo "$h" | awk '/^Version/ {print; exit}')" || bad "--help: $(echo "$h" | head -n 3)"
echo "$h" | has "--resolution" && echo "$h" | has "--windowed" && ok "--help lists --resolution and --windowed" || bad "--help options"
mkdir -p .emulationstation roms/qnx
cat > .emulationstation/es_systems.cfg <<EOF
<systemList>
  <system>
    <name>qnx</name>
    <fullname>QNX Test System</fullname>
    <path>$T/roms/qnx</path>
    <extension>.sh</extension>
    <command>sh %ROM%</command>
    <platform>pc</platform>
    <theme>qnx</theme>
  </system>
</systemList>
EOF
echo 'echo game' > roms/qnx/game.sh
to 120 "$ES" --home "$T" --windowed --resolution 320 240 --no-splash > run.log 2>&1; rc=$?
LOG=.emulationstation/es_log.txt
[ -f $LOG ] && has "EmulationStation - v" < $LOG && ok "started: es_log.txt has '$(awk '/EmulationStation - v/ {sub(/^.*EmulationStation/, "EmulationStation"); print; exit}' $LOG)'" || { bad "no start log"; tail -n 5 run.log; }
[ $rc -ne 124 ] && ok "stopped by itself without a display (rc=$rc)" || bad "hung (killed after 120 s)"
info "last log line: $(tail -n 1 $LOG 2>/dev/null)"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
