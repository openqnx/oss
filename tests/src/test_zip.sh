#!/bin/sh
# zip: archives a small tree (a stored file, a compressible file, a
# subdirectory) and checks the archive itself: the local header signature
# PK\3\4, the CRC-32 of the stored "abc" entry (352441c2) and its data
# verbatim; deflate shrinks 74 KiB of numbered lines below 8 KiB; -sf
# lists the entries, -d deletes one, -u adds a changed file, zipnote lists
# and edits entry comments; zipsplit splits an archive into parts no larger
# than the limit. (There is no unzip on the target: the archive is
# inspected byte-wise and with zip's own listing.)
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
N=zip
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
Z=$L/bin/zip
r=$("$Z" -v 2>&1 | sed -n 2p); echo "$r" | has "Zip 3.0 (July 5th 2008)" && ok "zip -v: Zip 3.0" || bad "version: $r"
"$Z" -v 2>&1 | has "LARGE_FILE_SUPPORT" && "$Z" -v 2>&1 | has "ZIP64_SUPPORT" && ok "built with LARGE_FILE_SUPPORT and ZIP64_SUPPORT" || bad "large file support missing"
mkdir -p tree/sub; printf 'abc' > tree/abc.txt
awk 'BEGIN { for (i = 0; i < 1000; i++) printf "QNX Neutrino real-time OS, line %06d\n", i }' > tree/rep.txt; cp tree/rep.txt tree/sub/rep2.txt
printf 'inner file\n' > tree/sub/inner.txt
r=$(cd tree && to 60 "$Z" -0 ../s.zip abc.txt 2>&1); rc=$?
sig=$(od -A n -t x1 -N 4 s.zip | tr -d ' '); crc=$(od -A n -t x1 -j 14 -N 4 s.zip | tr -d ' ')
[ $rc -eq 0 ] && [ "$sig" = 504b0304 ] && [ "$crc" = c2412435 ] && ok "stored abc.txt: PK\\3\\4 header, CRC-32 352441c2 in it" || bad "s.zip (rc $rc): sig $sig crc $crc $r"
grep -q abc s.zip && ok "stored (-0): the data is in the archive verbatim" || bad "stored data"
r=$(cd tree && to 60 "$Z" -r -9 ../t.zip . 2>&1); rc=$?
s=$(ls -l t.zip | awk '{print $5}')
[ $rc -eq 0 ] && [ "$s" -lt 8000 ] && echo "$r" | has "rep.txt (deflated" && ok "deflate -9: two 37 KiB text files, archive $s bytes ($(echo "$r" | grep rep.txt | sed 's/.*(//; s/)//'))" || bad "t.zip (rc $rc, $s bytes): $r"
r=$("$Z" -sf t.zip 2>&1 | grep -v -E '^(Archive contains|Total )' | sed 's/^ *//' | sort | tr '\n' ' ')
[ "$r" = "abc.txt rep.txt sub/ sub/inner.txt sub/rep2.txt " ] && ok "-sf lists abc.txt rep.txt sub/ sub/inner.txt sub/rep2.txt" || bad "-sf: $r"
"$Z" -q -d t.zip rep.txt && r=$("$Z" -sf t.zip 2>&1 | grep -c rep.txt)
[ "$r" = 0 ] && ok "-d removes rep.txt" || bad "-d: $r"
sleep 1; printf 'abcd' > tree/abc.txt
r=$(cd tree && "$Z" -u ../t.zip abc.txt 2>&1); echo "$r" | has "updating: abc.txt" && ok "-u: abc.txt updated" || bad "-u: $r"
"$L/bin/zipnote" t.zip | awk '{print} $0 == "@ abc.txt" {print "the first file"} END {print "QNX archive"}' > notes
"$L/bin/zipnote" -w t.zip < notes; r=$("$L/bin/zipnote" t.zip 2>&1)
echo "$r" | has "the first file" && echo "$r" | has "QNX archive" && ok "zipnote -w: entry and archive comments written and read back" || bad "zipnote: $r"
(cd tree && "$Z" -q -r -0 ../big.zip .)
r=$("$L/bin/zipsplit" -n 40000 big.zip 2>&1); rc=$?
n=$(ls big?.zip 2>/dev/null | wc -l | tr -d ' '); mx=$(ls -l big?.zip 2>/dev/null | awk '$5 > m {m = $5} END {print m+0}')
[ $rc -eq 0 ] && [ "$n" -ge 2 ] && [ "$mx" -le 40000 ] && ok "zipsplit -n 40000: $n parts, largest $mx bytes" || bad "zipsplit (rc $rc): $n parts, max $mx: $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
