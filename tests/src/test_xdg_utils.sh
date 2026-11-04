#!/bin/sh
# xdg-utils (generic desktop): xdg-mime sets and queries the default
# application for a MIME type through mimeapps.list and installs a MIME
# package; xdg-icon-resource installs an icon into the hicolor theme;
# xdg-desktop-menu installs a menu entry; xdg-open reports its version.
# Everything goes to private XDG directories in a temp dir.
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
N=xdg-utils
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
fhas() { awk -v p="$2" 'index($0, p) {f=1} END {exit !f}' "$1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
B=$L/bin
T=$(mktemp -d /tmp/qnxport-xdg.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
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
HOME=$T; XDG_DATA_HOME=$T/data; XDG_CONFIG_HOME=$T/config; XDG_DATA_DIRS=$T/sysdata; XDG_CONFIG_DIRS=$T/sysconfig
export HOME XDG_DATA_HOME XDG_CONFIG_HOME XDG_DATA_DIRS XDG_CONFIG_DIRS
unset XDG_CURRENT_DESKTOP DESKTOP_SESSION KDE_FULL_SESSION GNOME_DESKTOP_SESSION_ID XDG_SESSION_DESKTOP DISPLAY WAYLAND_DISPLAY BROWSER
mkdir -p data/applications config sysdata sysconfig
r=$(sh "$B/xdg-open" --version 2>&1); case "$r" in "xdg-open "[0-9]*) ok "xdg-open --version: $r" ;; *) bad "xdg-open --version: $r" ;; esac
printf '[Desktop Entry]\nType=Application\nName=QNX Viewer\nExec=sh %%f\nMimeType=text/x-qnx;\n' > data/applications/qnxview.desktop
sh "$B/xdg-mime" default qnxview.desktop text/x-qnx 2> d.err && fhas config/mimeapps.list "text/x-qnx=qnxview.desktop" && fhas config/mimeapps.list "[Default Applications]" \
  && ok "xdg-mime default: mimeapps.list has text/x-qnx=qnxview.desktop" || { bad "xdg-mime default"; cat d.err; cat config/mimeapps.list 2>/dev/null; }
r=$(sh "$B/xdg-mime" query default text/x-qnx 2> q.err)
[ "$r" = qnxview.desktop ] && ok "xdg-mime query default text/x-qnx: $r" || bad "xdg-mime query default: $r $(cat q.err)"
[ -s q.err ] && info "xdg-mime stderr: $(head -n 1 q.err) (toybox realpath on a /proc/boot program)"
r=$(sh "$B/xdg-mime" query default text/x-unknown-qnx 2>/dev/null); [ -z "$r" ] && ok "query default for an unknown type: empty" || bad "unknown type: $r"
cat > qnx-test.xml <<'EOF'
<?xml version="1.0"?>
<mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">
  <mime-type type="text/x-qnx"><comment>QNX text</comment><glob pattern="*.qnx"/></mime-type>
</mime-info>
EOF
sh "$B/xdg-mime" install --mode user --novendor qnx-test.xml 2> m.err && [ -f data/mime/packages/qnx-test.xml ] && ok "xdg-mime install --mode user: package copied" || { bad "xdg-mime install"; cat m.err; }
printf '\211PNG\r\n\032\n' > icon.png
sh "$B/xdg-icon-resource" install --mode user --size 16 --novendor icon.png qnxicon 2> i.err && cmp -s icon.png data/icons/hicolor/16x16/apps/qnxicon.png && ok "xdg-icon-resource: hicolor/16x16/apps/qnxicon.png" || { bad "xdg-icon-resource"; cat i.err; }
printf '[Desktop Entry]\nType=Application\nName=QNX Menu Test\nExec=sh\nCategories=Utility;\n' > qnx-menutest.desktop
sh "$B/xdg-desktop-menu" install --mode user qnx-menutest.desktop 2> dm.err && [ -f data/applications/qnx-menutest.desktop ] && ok "xdg-desktop-menu install: entry in applications/" || { bad "xdg-desktop-menu install"; cat dm.err; }
sh "$B/xdg-desktop-menu" uninstall --mode user qnx-menutest.desktop 2>/dev/null && [ ! -f data/applications/qnx-menutest.desktop ] && ok "xdg-desktop-menu uninstall" || bad "xdg-desktop-menu uninstall"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
