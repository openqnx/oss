#!/bin/sh
# desktop-file-utils: desktop-file-validate accepts a correct .desktop file
# and rejects broken ones with the reason; desktop-file-install copies one
# into a directory while editing keys; desktop-file-edit changes a key in
# place; update-desktop-database builds mimeinfo.cache from MimeType=.
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
N=desktop-file-utils
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
fhas() { awk -v p="$2" 'index($0, p) {f=1} END {exit !f}' "$1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
B=$L/bin
T=$(mktemp -d /tmp/qnxport-dfu.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
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
info "$("$B/desktop-file-install" --version 2>&1 | head -n 1)"
cat > qnxview.desktop <<'EOF'
[Desktop Entry]
Type=Application
Name=QNX Viewer
Comment=Views QNX test files
Exec=qnxview %f
Icon=qnxview
Terminal=false
Categories=Utility;
MimeType=application/x-qnx-test;text/x-qnx-log;
EOF
r=$("$B/desktop-file-validate" qnxview.desktop 2>&1) && [ -z "$r" ] && ok "validate: correct file accepted" || bad "validate correct file: $r"
sed '/^Type=/d' qnxview.desktop > notype.desktop
r=$("$B/desktop-file-validate" notype.desktop 2>&1) && bad "validate accepted a file without Type" || { case "$r" in *Type*) ok "validate: missing Type rejected" ;; *) bad "missing Type message: $r" ;; esac; }
sed 's/^Categories=Utility;/Categories=NotARealCategory;/' qnxview.desktop > cat.desktop
r=$("$B/desktop-file-validate" cat.desktop 2>&1)
case "$r" in *NotARealCategory*) ok "validate: unregistered category reported" ;; *) bad "category check: $r" ;; esac
mkdir apps
"$B/desktop-file-install" --dir="$T/apps" --set-name="QNX Viewer Pro" --add-category=Development --remove-key=Comment qnxview.desktop 2> i.err \
  && f=apps/qnxview.desktop && fhas $f "Name=QNX Viewer Pro" && fhas $f "Development;" && ! fhas $f "Comment=" \
  && ok "install: copied with --set-name, --add-category, --remove-key" || { bad "desktop-file-install"; cat i.err; }
"$B/desktop-file-edit" --set-key=Terminal --set-value=true apps/qnxview.desktop 2> e.err && fhas apps/qnxview.desktop "Terminal=true" && ok "edit: Terminal=true set in place" || { bad "desktop-file-edit"; cat e.err; }
"$B/desktop-file-validate" apps/qnxview.desktop >/dev/null 2>&1 && ok "edited file still validates" || bad "edited file invalid"
"$B/update-desktop-database" "$T/apps" 2> u.err && fhas apps/mimeinfo.cache "[MIME Cache]" && fhas apps/mimeinfo.cache "application/x-qnx-test=qnxview.desktop;" && fhas apps/mimeinfo.cache "text/x-qnx-log=qnxview.desktop;" \
  && ok "update-desktop-database: mimeinfo.cache maps both MIME types" || { bad "update-desktop-database"; cat u.err; cat apps/mimeinfo.cache 2>/dev/null; }
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
