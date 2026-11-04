#!/bin/sh
# graphviz: lay out a known digraph (6 nodes, 6 edges, one cluster) with
# dot and render it: SVG (node, edge, cluster counts, the edge label), PNG
# (signature, size matching the -Tplain bounding box at 96 dpi), the plain
# layout (rankdir=LR order), neato/circo/fdp/twopi, GIF through gd; gc and
# acyclic.
# Runs in the background with its output to a log, for at most 170 s (a hang
# ends as a FAIL with what it printed so far).
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
N=graphviz
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
D=$(cd "$(dirname "$0")" && pwd)   # helpers next to this test
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
G=$L/bin
info "$("$G/dot" -V 2>&1)"
# The plugin configuration is written at install time (dot -c); a package
# installed without it reports no formats.
cat > g.dot <<'EOS'
digraph G {
  rankdir=LR;
  a -> b -> c;
  a -> c [label="skip"];
  c -> d;
  b -> d;
  subgraph cluster_x { label="X"; e; f; }
  e -> f;
}
EOS
to 60 "$G/dot" -Tsvg g.dot -o g.svg 2> err.txt
nodes=$(grep -c 'class="node"' g.svg 2>/dev/null); edges=$(grep -c 'class="edge"' g.svg 2>/dev/null)
clusters=$(grep -c 'class="cluster"' g.svg 2>/dev/null)
[ "$nodes" = 6 ] && [ "$edges" = 6 ] && [ "$clusters" = 1 ] && ok "dot -Tsvg: 6 nodes, 6 edges, 1 cluster" \
  || { bad "dot -Tsvg: $nodes nodes, $edges edges, $clusters clusters"; cat err.txt; }
grep -q '<svg' g.svg && grep -q '>skip<' g.svg && ok "SVG with the edge label 'skip'" || bad "SVG content"
to 60 "$G/dot" -Tpng g.dot -o g.png 2> err.txt
sig=$(od -An -tx1 -N8 g.png 2>/dev/null | tr -d ' \n')
[ "$sig" = 89504e470d0a1a0a ] && ok "dot -Tpng: a PNG file ($(wc -c < g.png | tr -d ' ') bytes)" || { bad "dot -Tpng: signature '$sig'"; cat err.txt; }
# IHDR width/height (big-endian at offsets 16 and 20) against the layout's
# bounding box (-Tplain: 'graph scale width height' in inches) plus the
# default pad (4 pt a side), at the PNG renderer's 96 dpi
w=$(od -An -tu1 -j16 -N4 g.png | awk '{print $1*16777216+$2*65536+$3*256+$4}')
h=$(od -An -tu1 -j20 -N4 g.png | awk '{print $1*16777216+$2*65536+$3*256+$4}')
"$G/dot" -Tplain g.dot > g.plain
pw=$(awk '/^graph/{printf "%d", ($3*72+8)*96/72+0.5}' g.plain); ph=$(awk '/^graph/{printf "%d", ($4*72+8)*96/72+0.5}' g.plain)
info "PNG ${w}x${h}, layout + pad at 96 dpi ${pw}x${ph} px"
[ -n "$w" ] && [ "$w" -ge $((pw - 3)) ] && [ "$w" -le $((pw + 3)) ] && [ "$h" -ge $((ph - 3)) ] && [ "$h" -le $((ph + 3)) ] \
  && ok "PNG size matches the layout (cairo renderer)" || bad "PNG size ${w}x${h} vs ${pw}x${ph}"
[ "$(grep -c '^node' g.plain)" = 6 ] && [ "$(grep -c '^edge' g.plain)" = 6 ] && ok "-Tplain: 6 node and 6 edge lines" || bad "-Tplain"
# rankdir=LR: a left of b left of c
ax=$(awk '$1=="node"&&$2=="a"{print $3}' g.plain); cx=$(awk '$1=="node"&&$2=="c"{print $3}' g.plain)
awk -v a="$ax" -v c="$cx" 'BEGIN{exit !(a+0 < c+0)}' && ok "rankdir=LR: a is left of c" || bad "a at $ax, c at $cx"
# other layout engines and the gd renderer
for e in neato circo fdp twopi; do
  to 60 "$G/$e" -Tsvg g.dot 2>/dev/null | grep -c 'class="node"' | grep -qx 6 && ok "$e: 6 nodes" || bad "$e"
done
to 60 "$G/dot" -Tgif g.dot -o g.gif 2> err.txt
[ "$(head -c 3 g.gif 2>/dev/null)" = GIF ] && ok "dot -Tgif (the gd renderer)" || { bad "-Tgif"; cat err.txt; }
# gvpr and the graph tools
"$G/gc" -n -e g.dot 2>/dev/null | grep -q '6 *6' && ok "gc: 6 nodes, 6 edges" || bad "gc: $("$G/gc" -n -e g.dot 2>&1)"
printf 'digraph { x -> y; y -> x; }' | "$G/acyclic" -n 2>/dev/null; [ $? = 1 ] && ok "acyclic: the cycle is found" || bad "acyclic"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
