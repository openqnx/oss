#!/bin/sh
# GNU tar: create, list, extract and compare a small tree (files, a symlink,
# an empty directory, a long path), with gzip, --exclude, --transform,
# appending (-r), -d (diff against the file system) and the POSIX format.
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
N=tar
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
TR=$L/bin/tar
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
info "$("$TR" --version 2>&1 | head -n 1)"
mkdir -p src/dir/empty src/very/long/path/name/that/goes/on/and/on/beyond/one/hundred/characters/in/total/length
echo "hello qnx" > src/a.txt; seq 1 2000 > src/dir/numbers.txt 2>/dev/null || { i=0; while [ $i -lt 2000 ]; do echo $i; i=$((i+1)); done > src/dir/numbers.txt; }
echo deep > src/very/long/path/name/that/goes/on/and/on/beyond/one/hundred/characters/in/total/length/deep.txt
echo skip > src/skip.log; ln -s a.txt src/link.txt
"$TR" -cf t.tar src && ok "-cf created t.tar ($(wc -c < t.tar | tr -d ' ') bytes)" || bad "-cf"
n=$("$TR" -tf t.tar | wc -l | tr -d ' ')
[ "$n" -ge 20 ] && ok "-tf lists $n members" || bad "-tf lists $n members"
mkdir x && "$TR" -xf t.tar -C x && diff -r src x/src >/dev/null && ok "-xf: extracted tree identical (diff -r)" || bad "-xf: tree differs"
[ -L x/src/link.txt ] && [ "$(cat x/src/link.txt)" = "hello qnx" ] && ok "symlink restored" || bad "symlink"
[ -d x/src/dir/empty ] && ok "empty directory restored" || bad "empty directory"
"$TR" -czf t.tgz --exclude='*.log' src && [ "$(od -An -tx1 -N2 t.tgz | tr -d ' ')" = 1f8b ] && ok "-czf: gzip magic" || bad "-czf"
"$TR" -tzf t.tgz | awk '/skip\.log/ {f=1} END {exit f}' && ok "--exclude='*.log' left skip.log out" || bad "--exclude"
mkdir y && "$TR" -xzf t.tgz -C y && cmp -s y/src/dir/numbers.txt src/dir/numbers.txt && ok "-xzf: numbers.txt identical" || bad "-xzf"
"$TR" -cf tr.tar --transform='s,^src,renamed,' src/a.txt && [ "$("$TR" -tf tr.tar)" = renamed/a.txt ] && ok "--transform renames members" || bad "--transform: $("$TR" -tf tr.tar 2>&1)"
echo extra > extra.txt; "$TR" -rf t.tar extra.txt && "$TR" -tf t.tar | awk '$0=="extra.txt"{f=1} END{exit !f}' && ok "-rf appended extra.txt" || bad "-rf append"
"$TR" -df t.tar >/dev/null 2>&1 && ok "-df: archive matches file system" || bad "-df reported differences"
echo "hello CHANGED" > src/a.txt
"$TR" -df t.tar > d.out 2>&1; awk '/a\.txt/ {f=1} END {exit !f}' d.out && ok "-df detects the changed file" || { bad "-df missed the change"; cat d.out; }
"$TR" --format=posix -cf p.tar src/a.txt && [ "$(od -An -tx1 -j257 -N5 p.tar | tr -d " \n")" = 7573746172 ] && ok "--format=posix: ustar magic" || bad "posix format"
r=$("$TR" -xOf t.tar src/very/long/path/name/that/goes/on/and/on/beyond/one/hundred/characters/in/total/length/deep.txt 2>&1)
[ "$r" = deep ] && ok "-xO of a >100-char path" || bad "long path: $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
