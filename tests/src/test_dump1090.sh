#!/bin/sh
# dump1090 decodes Mode S / ADS-B from a recorded I/Q file (--ifile), no
# radio needed: the test synthesises 2 MHz 8-bit I/Q samples of a known
# extended squitter (8D4840D6202CC371C32CE0576098, ICAO 4840D6, callsign
# KLM1023) -- preamble pulses plus pulse-position-modulated bits -- and
# dump1090 must demodulate it, pass the CRC and decode the identification.
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
N=dump1090
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index(tolower($0), tolower(p)) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
DU=$L/bin/dump1090
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
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
MSG=8d4840d6202cc371c32ce0576098
# One sample = one I/Q byte pair. High = I 255, Q 127 (magnitude 128); low = 127,127 (0).
# Preamble: pulses at samples 0, 2, 7, 9 of 16; then each bit is 2 samples: 1 = high,low; 0 = low,high.
awk -v m="$MSG" 'BEGIN {
  H = "\\377\\177"; Z = "\\177\\177"; pad = ""
  for (i = 0; i < 400; i++) pad = pad Z
  s = pad
  split("1 0 1 0 0 0 0 1 0 1 0 0 0 0 0 0", pre, " ")
  for (i = 1; i <= 16; i++) s = s (pre[i] ? H : Z)
  hex = "0123456789abcdef"
  for (i = 1; i <= length(m); i++) {
    v = index(hex, substr(m, i, 1)) - 1
    for (b = 3; b >= 0; b--) { bit = int(v / (2 ^ b)) % 2; s = s (bit ? H Z : Z H) }
  }
  printf "%s", s pad pad
}' > iq.esc
printf "$(cat iq.esc)" > iq.bin
sz=$(wc -c < iq.bin | tr -d ' ')
[ "$sz" = $(( (400 * 3 + 16 + 224) * 2 )) ] && ok "synthesised I/Q file: $sz bytes" || bad "I/Q file size $sz"
r=$(to 60 "$DU" --ifile iq.bin --raw 2>&1)
case "$r" in *"*$MSG;"*) ok "--raw: *$MSG; demodulated" ;; *) bad "--raw: $(echo "$r" | head -3)" ;; esac
r=$(to 60 "$DU" --ifile iq.bin 2>&1)
echo "$r" | has "4840d6" && ok "decoded ICAO address 4840D6" || { bad "ICAO address"; echo "$r" | head -12; }
echo "$r" | has "KLM1023" && ok "decoded identification KLM1023" || bad "identification KLM1023"
echo "$r" | awk '/CRC/ && /ok/ {f=1} END {exit !f}' && ok "CRC ok" || bad "CRC"
r=$(to 60 "$DU" --ifile iq.bin --stats 2>&1)
echo "$r" | awk '/valid preambles/ && $1 >= 1 {a=1} /total usable messages/ && $1 >= 1 {b=1} END {exit !(a && b)}' && ok "--stats: valid preamble, usable message counted" || { bad "--stats"; echo "$r" | head -12; }
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
