#!/bin/sh
# itstool: extract translatable strings from an XML document into a PO
# template (ITS rules: a translate="no" element is skipped), then merge a
# French translation (compiled with msgfmt) back into a translated document.
# itstool is a Python script that imports libxml2's Python bindings; this is
# checked first and reported plainly when they are missing.
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
N=itstool
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
fhas() { awk -v p="$2" 'index($0, p) {f=1} END {exit !f}' "$1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
PY=$L/bin/python3; [ -x "$PY" ] || PY=$(command -v python3)
IT="$L/bin/itstool"
T=$(mktemp -d /tmp/qnxport-$N.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
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
h=$(head -n 1 "$L/bin/itstool"); case "$h" in "#!$L/bin/python3"*|"#!/usr/local/bin/python3"*) ok "#! line: $h" ;; *) bad "#! line is not the installed python3: $h" ;; esac
r=$("$PY" -c 'import libxml2; print("libxml2", libxml2.__name__)' 2>&1)
case "$r" in "libxml2 libxml2") ok "python3 imports libxml2 (the bindings itstool needs)" ;; *) bad "python3 cannot import libxml2: $(echo "$r" | tail -n 1)" ;; esac
v=$(to 30 $IT --version 2>&1); case "$v" in *"itstool "[0-9]*) ok "--version: $(echo "$v" | tail -n 1)" ;; *) bad "--version: $(echo "$v" | tail -n 1)" ;; esac
cat > doc.xml <<'EOF'
<?xml version="1.0" encoding="utf-8"?>
<doc xmlns:its="http://www.w3.org/2005/11/its" its:version="2.0">
  <title>Hello QNX</title>
  <para>The microkernel is small.</para>
  <code its:translate="no">procnto-smp-instr</code>
</doc>
EOF
$IT -o doc.pot doc.xml > x.log 2>&1 && ok "extracted doc.pot" || { bad "extract"; tail -2 x.log; }
fhas doc.pot 'msgid "Hello QNX"' && fhas doc.pot 'msgid "The microkernel is small."' && ok "POT has the title and the paragraph" || bad "POT messages"
fhas doc.pot 'procnto-smp-instr' && bad "translate=no element was extracted" || ok "its:translate=\"no\" element skipped"
cat > fr.po <<'EOF'
msgid ""
msgstr ""
"Content-Type: text/plain; charset=UTF-8\n"

msgid "Hello QNX"
msgstr "Bonjour QNX"

msgid "The microkernel is small."
msgstr "Le micro-noyau est petit."
EOF
if "$L/bin/msgfmt" -o fr.mo fr.po 2> mf.err || msgfmt -o fr.mo fr.po 2>> mf.err; then
  $IT -m fr.mo -l fr -o doc.fr.xml doc.xml > m.log 2>&1 && fhas doc.fr.xml "<title>Bonjour QNX</title>" && fhas doc.fr.xml "Le micro-noyau est petit." && fhas doc.fr.xml "procnto-smp-instr" \
    && ok "merged French translation, untranslatable code kept" || { bad "merge"; tail -2 m.log; }
else bad "msgfmt (gettext) could not compile fr.po"; cat mf.err; fi
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
