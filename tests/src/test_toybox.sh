#!/bin/sh
# Upstream toybox as the single binary /usr/local/opt/toybox/bin/toybox
# (QNX's own toybox is not touched): applets run as "toybox <applet>" with
# known output -- printf, seq, sort/uniq, sed, grep, cut, wc, od, xxd,
# base64, md5sum/sha256sum (FIPS vectors), factor, date -u, test;
# files: mkdir -p, cp -r, find -name, realpath, ls, stat, tar
# round trip, ln -s and readlink, and timeout's exit status.
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
N=toybox
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
TB=$L/opt/toybox/bin/toybox
t() { to 30 "$TB" "$@" 2>&1; }
r=$(t --version); [ "$r" = "toybox 0.8.15" ] && ok "toybox --version: 0.8.15" || bad "--version: $r"
r=$(t | tr ' ' '\n' | grep -c .); [ "$r" -ge 100 ] 2>/dev/null && ok "$r applets in the multicall binary" || bad "applet list: $r"
r=$(t printf '%05.2f|%x|%s|%c\n' 3.14159 255 qnx Z); [ "$r" = "03.14|ff|qnx|Z" ] && ok "printf: 03.14|ff|qnx|Z" || bad "printf: $r"
r=$(t seq -s: 1 3 7); [ "$r" = "1:4:7" ] && ok "seq -s: 1 3 7" || bad "seq: $r"
r=$(printf '3\n1\n2\n3\n10\n' | t sort -n | "$TB" uniq -c | "$TB" sed 's/^ *//' | tr '\n' ';'); [ "$r" = "1 1;1 2;2 3;1 10;" ] && ok "sort -n | uniq -c" || bad "sort|uniq: $r"
r=$(echo hello world | t sed -e 's/l/L/g' -e 's/\(w[a-z]*\)/[\1]/'); [ "$r" = "heLLo [wor]Ld" ] && ok "sed: s///g and a back-reference" || bad "sed: $r"
r=$(printf 'a:1\nb:22\nc:333\n' | t cut -d: -f2 | "$TB" wc -c | tr -d ' '); [ "$r" = 9 ] && ok "cut -d: -f2 | wc -c = 9" || bad "cut|wc: $r"
r=$(printf 'alpha\nbeta\ngamma\nalphabet\n' | t grep -c '^alpha'); [ "$r" = 2 ] && ok "grep -c" || bad "grep: $r"
r=$(printf 'QNX' | t od -An -tx1 | tr -d ' \n'); [ "$r" = "514e58" ] && ok "od -tx1" || bad "od: $r"
r=$(printf 'QNX\n' | t xxd -p); [ "$r" = "514e580a" ] && ok "xxd -p" || bad "xxd: $r"
r=$(printf 'foobar' | t base64); [ "$r" = "Zm9vYmFy" ] && ok "base64 foobar" || bad "base64: $r"
r=$(printf 'Zm9vYmFy' | t base64 -d); [ "$r" = "foobar" ] && ok "base64 -d" || bad "base64 -d: $r"
r=$(printf 'abc' | t md5sum | cut -d' ' -f1); [ "$r" = "900150983cd24fb0d6963f7d28e17f72" ] && ok "md5sum abc (RFC 1321)" || bad "md5sum: $r"
r=$(printf 'abc' | t sha256sum | cut -d' ' -f1); [ "$r" = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" ] && ok "sha256sum abc (FIPS 180-2)" || bad "sha256sum: $r"
r=$(t factor 360 1000003); [ "$r" = "360: 2 2 2 3 3 5
1000003: 1000003" ] && ok "factor 360 1000003" || bad "factor: $r"
r=$(t date -u -d @86400 +%Y-%m-%d); [ "$r" = "1970-01-02" ] && ok "date -u -d @86400" || bad "date: $r"
t test 12 -gt 9 && ! t test -z x && ok "test 12 -gt 9, ! test -z x" || bad "test"
t mkdir -p d/a/b && echo one > d/a/b/f1 && echo two > d/a/f2 && t ln -s a/b/f1 d/link
r=$(cd d && t find . -name 'f*' | "$TB" sort | tr '\n' ' '); [ "$r" = "./a/b/f1 ./a/f2 " ] && ok "find -name (directory walk)" || bad "find: $r"
r=$(cd d && t find . -type l); [ "$r" = "./link" ] && ok "find -type l (entry types without dirent d_type)" || bad "find -type l: $r"
r=$(t readlink d/link); [ "$r" = "a/b/f1" ] && ok "ln -s / readlink" || bad "readlink: $r"
r=$(t realpath d/a/../a/b/f1); [ "$r" = "$(cd d/a/b && pwd -P)/f1" ] && ok "realpath through .. and subdirectories" || bad "realpath: $r"
r=$(t ls -1 d/a | tr '\n' ' '); [ "$r" = "b f2 " ] && ok "ls -1" || bad "ls: $r"
t cp -r d e && cmp -s d/a/b/f1 e/a/b/f1 && [ "$(t readlink e/link)" = "a/b/f1" ] && ok "cp -r (symlink kept)" || bad "cp -r"
r=$(t stat -c '%s %F' d/a/f2); [ "$r" = "4 regular file" ] && ok "stat -c '%s %F'" || bad "stat: $r"
r=$(t stat -f -c %T .); [ -n "$r" ] && ok "stat -f: filesystem type $r" || bad "stat -f: $r"
t tar cf d.tar d && r=$(t tar tf d.tar | "$TB" sort | tr '\n' ' ')
case "$r" in "d/ d/a/ d/a/b/ d/a/b/f1 d/a/f2 d/link "*) ok "tar cf / tf" ;; *) bad "tar tf: $r" ;; esac
mkdir x && (cd x && t tar xf ../d.tar) && cmp -s x/d/a/f2 d/a/f2 && ok "tar xf round trip" || bad "tar xf"
t timeout 1 sleep 10; rc=$?; [ $rc -eq 124 ] && ok "timeout 1 sleep 10: exit 124" || bad "timeout: rc $rc"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
