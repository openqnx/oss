#!/bin/sh
# font-util: ucs2any re-encodes an ISO10646-1 BDF font into ISO8859-1 and
# ISO8859-2 with the installed mapping files (glyphs renumbered, registry
# rewritten, unmapped glyphs dropped); bdftruncate leaves glyphs at or above
# a threshold unencoded.
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
N=font-util
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
fhas() { awk -v p="$2" 'index($0, p) {f=1} END {exit !f}' "$1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
U=$L/share/fonts/X11/util
T=$(mktemp -d /tmp/qnxport-fontutil.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
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
glyph() { printf 'STARTCHAR %s\nENCODING %d\nSWIDTH 500 0\nDWIDTH 4 0\nBBX 4 4 0 0\nBITMAP\n90\n60\n60\n90\nENDCHAR\n' "$1" "$2"; }
{
  printf 'STARTFONT 2.1\nFONT -misc-qnxtest-medium-r-normal--4-40-75-75-c-40-ISO10646-1\nSIZE 4 75 75\nFONTBOUNDINGBOX 4 4 0 0\n'
  printf 'STARTPROPERTIES 2\nCHARSET_REGISTRY "ISO10646"\nCHARSET_ENCODING "1"\nENDPROPERTIES\nCHARS 5\n'
  glyph A 65; glyph eacute 233; glyph Lstroke 321; glyph uni3400 13312; glyph uni4E00 19968
  printf 'ENDFONT\n'
} > q.bdf
"$L/bin/ucs2any" q.bdf "$U/map-ISO8859-1" ISO8859-1 "$U/map-ISO8859-2" ISO8859-2 2> u.err
f1=q-ISO8859-1.bdf; f2=q-ISO8859-2.bdf
[ -f $f1 ] && [ -f $f2 ] && ok "ucs2any wrote $f1 and $f2" || { bad "ucs2any output files"; ls; head -3 u.err; }
fhas $f1 'CHARSET_REGISTRY "ISO8859"' && fhas $f1 'FONT -misc-qnxtest-medium-r-normal--4-40-75-75-c-40-ISO8859-1' && ok "ISO8859-1: registry and XLFD rewritten" || bad "ISO8859-1 registry"
[ "$(awk '/^CHARS / {print $2}' $f1)" = 2 ] && fhas $f1 "ENCODING 233" && ! fhas $f1 "Lstroke" && ok "ISO8859-1: A and eacute kept (233), Lstroke dropped" || bad "ISO8859-1 glyphs: $(awk '/^CHARS |^STARTCHAR/' $f1 | tr '\n' ' ')"
awk '/^STARTCHAR Lstroke/ {s=1} s && /^ENCODING/ {print $2; exit}' $f2 > enc
[ "$(cat enc)" = 163 ] && ok "ISO8859-2: Lstroke (U+0141) renumbered to 0xA3" || bad "ISO8859-2 Lstroke encoding: $(cat enc)"
"$L/bin/bdftruncate" 0x3200 < q.bdf > t.bdf 2> t.err
e=$(awk '/^STARTCHAR/ {n=$2} /^ENCODING/ {printf "%s=%s ", n, $2}' t.bdf)
[ "$e" = "A=65 eacute=233 Lstroke=321 uni3400=-1 uni4E00=-1 " ] && ok "bdftruncate 0x3200: glyphs >= U+3200 unencoded (-1), the rest kept" || { bad "bdftruncate: $e"; head -3 t.err; }
fhas t.bdf "ENDFONT" && ok "truncated font is complete (ENDFONT)" || bad "truncated font incomplete"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
