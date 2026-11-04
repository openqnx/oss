#!/bin/sh
# MAME without a display: its driver database queried from the command line
# -- -version, -listfull, -listxml (CPU, ROM set, screen of Pac-Man),
# -listroms, -listclones, -listsource, -verifyroms reporting the missing
# ROM set, and -validate running the driver validity checks for one driver.
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
N=mame
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
M=$L/bin/mame
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
HOME=$T; export HOME; SDL_VIDEODRIVER=dummy; SDL_AUDIODRIVER=dummy; export SDL_VIDEODRIVER SDL_AUDIODRIVER
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
mm() { to 100 "$M" -noreadconfig -rompath "$T/roms" "$@" 2>&1; }
mkdir roms
v=$(mm -version | head -n 1); case "$v" in 0.2[0-9][0-9]*) ok "-version: $v" ;; *) bad "-version: $v" ;; esac
r=$(mm -listfull pacman); echo "$r" | awk '$1 == "pacman" && /Pac-Man/ {f=1} END {exit !f}' && ok "-listfull pacman: $(echo "$r" | awk '$1=="pacman" {print; exit}')" || bad "-listfull: $r"
mm -listxml pacman > p.xml
has '<machine name="pacman"' < p.xml && ok "-listxml: <machine name=\"pacman\">" || { bad "-listxml machine"; head -c 300 p.xml; }
has 'name="Zilog Z80"' < p.xml && ok "-listxml: Zilog Z80 CPU" || bad "-listxml CPU"
has '<rom name="pacman.6e"' < p.xml && ok "-listxml: ROM pacman.6e" || bad "-listxml ROM"
awk '/<display / && /width="288"/ && /height="224"/ {f=1} END {exit !f}' p.xml && ok "-listxml: 288x224 display" || bad "-listxml display"
r=$(mm -listroms pacman); echo "$r" | has "pacman.6e" && ok "-listroms pacman lists pacman.6e" || bad "-listroms: $(echo "$r" | head -n 3)"
r=$(mm -listclones puckman); echo "$r" | awk '$1 == "pacman" && $2 == "puckman" {f=1} END {exit !f}' && ok "-listclones: pacman is a clone of puckman" || bad "-listclones: $(echo "$r" | head -n 3)"
r=$(mm -listsource pacman); echo "$r" | has "pacman.cpp" && ok "-listsource: pacman.cpp" || bad "-listsource: $r"
r=$(mm -verifyroms pacman); rc=$?; [ $rc -ne 0 ] && echo "$r" | has "pacman" && ok "-verifyroms: missing ROM set reported (rc=$rc)" || bad "-verifyroms (rc=$rc): $(echo "$r" | tail -n 2)"
r=$(mm -validate pacman); rc=$?; [ $rc -eq 0 ] && ok "-validate pacman: driver validity checks pass" || bad "-validate (rc=$rc): $(echo "$r" | tail -n 3)"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
