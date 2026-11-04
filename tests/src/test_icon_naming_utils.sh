#!/bin/sh
# icon-naming-utils: in a theme's 32x32 directory with actions/document-new.png,
# actions/edit-copy.svg and places/folder.png, the target's
# icon-name-mapping (Perl, XML::Simple over XML::Parser) reads the installed
# legacy-icon-mapping.xml and adds the legacy names as symlinks: gtk-new.png,
# stock_new-text.png and filenew.png point to document-new.png, gtk-copy.svg
# to edit-copy.svg, gnome-fs-directory.png to folder.png; names whose
# icon is missing get no link; without -c it prints its usage.
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
N=icon_naming_utils
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
M=$L/libexec/icon-name-mapping
mkdir -p theme/32x32/actions theme/32x32/places && cd theme/32x32 || exit 1
echo png > actions/document-new.png; echo svg > actions/edit-copy.svg; echo png > places/folder.png
r=$(to 120 "$M" -c actions 2>&1); rc=$?
[ $rc -eq 0 ] && echo "$r" | has "Setting up icon mapping for: actions" && ok "icon-name-mapping -c actions ran" || bad "actions (rc $rc): $r"
l=""; for n in gtk-new stock_new-text filenew; do [ -L actions/$n.png ] && [ "$(ls -l actions/$n.png | sed 's/.*-> //')" = document-new.png ] || l="$l $n"; done
[ -z "$l" ] && ok "gtk-new.png, stock_new-text.png, filenew.png -> document-new.png" || bad "missing/wrong links:$l ($(ls actions | tr '\n' ' '))"
[ -L actions/gtk-copy.svg ] && [ "$(ls -l actions/gtk-copy.svg | sed 's/.*-> //')" = edit-copy.svg ] && ok "gtk-copy.svg -> edit-copy.svg (SVG icons too)" || bad "gtk-copy.svg"
[ ! -e actions/gtk-open.png ] && [ ! -e actions/fileopen.png ] && ok "no links for document-open (no icon)" || bad "links made for a missing icon"
r=$(to 120 "$M" -c places 2>&1)
[ -L places/gnome-fs-directory.png ] && ok "places: gnome-fs-directory.png -> folder.png" || bad "places: $(ls places | tr '\n' ' ') $r"
cd "$T"
r=$("$M" 2>&1); rc=$?
[ $rc -ne 0 ] && echo "$r" | has "-c, --context=<dirname>" && ok "no -c: usage, exit $rc" || bad "usage (rc $rc): $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
