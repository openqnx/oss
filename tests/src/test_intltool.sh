#!/bin/sh
# intltool: intltool-merge puts the German translation from po/de.po into
# a .desktop file (Name[de]=Hallo Welt, Comment[de]) and, through
# XML::Parser, into an XML file (xml:lang="de" elements); intltool-extract
# writes the translatable strings of the XML as N_() C strings; intltoolize
# copies po/Makefile.in.in into a project and aclocal finds
# IT_PROG_INTLTOOL in the installed intltool.m4.
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
N=intltool
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
export PATH="$L/bin:$PATH"
mkdir po
cat > po/de.po <<'EOP'
msgid ""
msgstr ""
"Content-Type: text/plain; charset=UTF-8\n"

msgid "Hello World"
msgstr "Hallo Welt"

msgid "A greeting program"
msgstr "Ein Begrüßungsprogramm"
EOP
echo de > po/LINGUAS
printf '[Desktop Entry]\n_Name=Hello World\n_Comment=A greeting program\nExec=hello\n' > hello.desktop.in
r=$(to 120 "$L/bin/intltool-merge" -d -u po hello.desktop.in hello.desktop 2>&1); rc=$?
m=""; for l in 'Name=Hello World' 'Name[de]=Hallo Welt' 'Comment[de]=Ein Begrüßungsprogramm' 'Exec=hello'; do grep -qxF "$l" hello.desktop 2>/dev/null || m="$m [$l]"; done
[ $rc -eq 0 ] && [ -z "$m" ] && ok "-d: hello.desktop with Name[de]=Hallo Welt and Comment[de]" || { bad "desktop merge (rc $rc), missing:$m $r"; od -c hello.desktop | tail -n 6; }
printf '<?xml version="1.0"?>\n<app>\n  <_name>Hello World</_name>\n  <id>hello</id>\n</app>\n' > app.xml.in
r=$(to 120 "$L/bin/intltool-merge" -x -u po app.xml.in app.xml 2>&1); rc=$?
[ $rc -eq 0 ] && grep -q '<name>Hello World</name>' app.xml && grep -q '<name xml:lang="de">Hallo Welt</name>' app.xml && grep -q '<id>hello</id>' app.xml \
  && ok "-x (XML::Parser): <name xml:lang=\"de\">Hallo Welt</name>" || { bad "xml merge (rc $rc): $r"; cat app.xml 2>/dev/null; }
r=$(to 120 "$L/bin/intltool-extract" --type=gettext/xml app.xml.in 2>&1); rc=$?
[ $rc -eq 0 ] && grep -qF 'char *s = N_("Hello World");' app.xml.in.h && ok "intltool-extract: app.xml.in.h with N_(\"Hello World\")" || bad "extract (rc $rc): $r $(cat app.xml.in.h 2>/dev/null)"
mkdir proj && cd proj && mkdir po
printf 'AC_INIT([p], [1])\nIT_PROG_INTLTOOL([0.35.0])\nAC_OUTPUT\n' > configure.ac
r=$(to 120 "$L/bin/intltoolize" --force 2>&1); rc=$?
[ $rc -eq 0 ] && [ -f po/Makefile.in.in ] && grep -q 'intltool' po/Makefile.in.in && ok "intltoolize: po/Makefile.in.in installed" || bad "intltoolize (rc $rc): $r"
r=$(to 120 aclocal 2>&1); rc=$?
[ $rc -eq 0 ] && grep -q 'AC_DEFUN(\[IT_PROG_INTLTOOL\]' aclocal.m4 && ok "aclocal found IT_PROG_INTLTOOL (share/aclocal/intltool.m4)" || bad "aclocal (rc $rc): $r"
cd "$T"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
