#!/bin/sh
# mkfontscale / mkfontdir: index a font directory -- a BDF font and a
# gzip-compressed BDF font are read through FreeType, and fonts.dir must list
# both with their XLFD names; fonts.scale stays empty (no scalable fonts).
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
N=mkfontscale
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
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
mkbdf() { # family
  printf 'STARTFONT 2.1\nFONT -misc-%s-medium-r-normal--8-80-75-75-c-50-ISO10646-1\nSIZE 8 75 75\nFONTBOUNDINGBOX 5 8 0 0\n' "$1"
  printf 'STARTPROPERTIES 7\nFOUNDRY "misc"\nFAMILY_NAME "%s"\nWEIGHT_NAME "medium"\nSLANT "R"\nPIXEL_SIZE 8\nCHARSET_REGISTRY "ISO10646"\nCHARSET_ENCODING "1"\nENDPROPERTIES\nCHARS 2\n' "$1"
  for g in "A 65" "B 66"; do set -- $g; printf 'STARTCHAR %s\nENCODING %d\nSWIDTH 625 0\nDWIDTH 5 0\nBBX 5 8 0 0\nBITMAP\n20\n50\n88\n88\nF8\n88\n88\n00\nENDCHAR\n' $1 $2; done
  printf 'ENDFONT\n'
}
mkdir fonts; mkbdf qnxone > fonts/qnxone.bdf; mkbdf qnxtwo > fonts/qnxtwo.bdf; gzip -9 fonts/qnxtwo.bdf
sh "$L/bin/mkfontdir" "$T/fonts" > m.log 2>&1 && [ -f fonts/fonts.dir ] && ok "mkfontdir wrote fonts.dir" || { bad "mkfontdir"; head -5 m.log; }
n=$(head -n 1 fonts/fonts.dir 2>/dev/null)
[ "$n" = 2 ] && ok "fonts.dir: 2 entries" || { bad "fonts.dir count: $n"; cat fonts/fonts.dir 2>/dev/null; }
LC_ALL=C awk 'tolower($0) ~ /^qnxone\.bdf -misc-qnxone-medium-r-normal--8-80-75-75-c-50-iso10646-1$/ {a=1} tolower($0) ~ /^qnxtwo\.bdf\.gz -misc-qnxtwo-medium-r-normal--8-80-75-75-c-50-iso10646-1$/ {b=1} END {exit !(a && b)}' fonts/fonts.dir 2>/dev/null \
  && ok "fonts.dir lists qnxone.bdf and qnxtwo.bdf.gz with their XLFDs" || { bad "fonts.dir entries"; cat fonts/fonts.dir 2>/dev/null; }
"$L/bin/mkfontscale" "$T/fonts" > s.log 2>&1 && [ "$(head -n 1 fonts/fonts.scale 2>/dev/null)" = 0 ] && ok "mkfontscale: fonts.scale with 0 scalable fonts" || { bad "mkfontscale"; head -3 s.log; cat fonts/fonts.scale 2>/dev/null | head -3; }
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
