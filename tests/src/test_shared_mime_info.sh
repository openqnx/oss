#!/bin/sh
# shared-mime-info: update-mime-database compiles a MIME database -- a
# custom package (glob, magic, sub-class, comment) into globs2, magic,
# subclasses, mime.cache and the per-type XML; and the shipped
# freedesktop.org.xml into a full database with the well-known types.
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
N=shared-mime-info
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
fhas() { LC_ALL=C awk -v p="$2" 'index($0, p) {f=1} END {exit !f}' "$1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
UM=$L/bin/update-mime-database
T=$(mktemp -d /tmp/qnxport-smi.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
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
info "$("$UM" -v 2>&1 | head -n 1)"
mkdir -p mime/packages
cat > mime/packages/qnx-test.xml <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">
  <mime-type type="application/x-qnx-test">
    <comment>QNX test file</comment>
    <sub-class-of type="text/plain"/>
    <glob pattern="*.qnxt"/>
    <magic priority="60"><match type="string" offset="0" value="QNXTEST"/></magic>
  </mime-type>
</mime-info>
EOF
"$UM" "$T/mime" 2> u.err && ok "update-mime-database on a custom package" || { bad "update-mime-database"; head -5 u.err; }
fhas mime/globs2 "application/x-qnx-test:*.qnxt" && ok "globs2: *.qnxt -> application/x-qnx-test" || bad "globs2"
strings mime/magic 2>/dev/null | awk '/QNXTEST/ {f=1} END {exit !f}' && [ "$(head -c 10 mime/magic 2>/dev/null)" = "MIME-Magic" ] && ok "magic: MIME-Magic file with the QNXTEST rule" || bad "magic"
fhas mime/subclasses "application/x-qnx-test text/plain" && ok "subclasses: x-qnx-test is text/plain" || bad "subclasses"
fhas mime/application/x-qnx-test.xml "QNX test file" && ok "per-type XML with the comment" || bad "per-type XML"
[ "$(od -An -tx1 -N4 mime/mime.cache 2>/dev/null | tr -d ' \n')" = 00010002 ] && ok "mime.cache: version 1.2 header" || bad "mime.cache header: $(od -An -tx1 -N4 mime/mime.cache 2>/dev/null)"
FD=$L/share/mime/packages/freedesktop.org.xml
if [ -f "$FD" ]; then
  mkdir -p full/packages && cp "$FD" full/packages/
  "$UM" "$T/full" 2> f.err && ok "full database from freedesktop.org.xml" || { bad "full database"; head -3 f.err; }
  fhas full/globs2 "image/png:*.png" && fhas full/globs2 "application/pdf:*.pdf" && ok "well-known globs: *.png, *.pdf" || bad "well-known globs"
  n=$(wc -l < full/types 2>/dev/null | tr -d ' '); [ "${n:-0}" -gt 500 ] && ok "types: $n MIME types" || bad "types: ${n:-0}"
else bad "freedesktop.org.xml not installed at $FD"; fi
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
