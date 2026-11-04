#!/bin/sh
# fio: real jobs on the target's file system, with known answers.
#  1. a sequential write of a 2 MiB file in 64 KiB blocks with
#     verify=crc32c (written blocks read back and checked: do_verify), then a
#     sequential read job that verifies the same blocks; the JSON report
#     gives 2048 KiB and 32 I/Os per direction and error 0, the file is
#     2097152 bytes and every block starts with fio's verify header magic
#     (0xacca);
#  2. one byte overwritten at offset 200000: the read job fails, and reports
#     the crc32c mismatch in the block at offset 196608 (3 * 64 KiB);
#  3. verify=pattern writes exactly the 32-bit pattern 0x5aa5c33c;
#  4. POSIX AIO (ioengine=posixaio, iodepth 8): random 4 KiB writes verified
#     with md5 (1 MiB); the mmap engine reads the file back with verification.
# fio runs its jobs as threads on QNX (built without System V shm).
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
N=fio
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
F=$L/bin/fio
info "$("$F" --version 2>&1 | head -n 1)"
cat > seq.fio <<'EOJ'
[global]
filename=fio.dat
size=2m
bs=64k
ioengine=psync
verify=crc32c
[seqwrite]
rw=write
do_verify=1
[seqread]
stonewall
rw=read
EOJ
to 120 "$F" --output-format=json --output=seq.json seq.fio > seq.log 2>&1; rc=$?
[ $rc -eq 0 ] && ok "sequential write + read with verify=crc32c: exit 0" || { bad "seq.fio rc $rc"; cat seq.log; cat seq.json; }
# The report: per job a read, a write and a trim section, in that order.
r=$(awk -F' : ' '/"jobname"/ {gsub(/[",]/, "", $2); j=$2; n=0}
                  /"error"/ {gsub(/,/, "", $2); print j, "error", $2}
                  /"io_kbytes"/ {gsub(/,/, "", $2); n++; print j, n, $2}' seq.json | tr '\n' ' ')
[ "$r" = "seqwrite error 0 seqwrite 1 2048 seqwrite 2 2048 seqwrite 3 0 seqread error 0 seqread 1 2048 seqread 2 0 seqread 3 0 " ] \
  && ok "report: seqwrite wrote 2048 KiB (and read them back to verify), seqread read 2048 KiB, error 0" || bad "report: $r"
n=$(grep -c '"total_ios" : 32,' seq.json)
[ "$n" -ge 3 ] && ok "report: 32 I/Os of 64 KiB per direction" || bad "total_ios 32: $n times"
s=$(ls -l fio.dat | awk '{print $5}')
[ "$s" = 2097152 ] && ok "fio.dat is 2097152 bytes" || bad "fio.dat size $s"
m=$(od -A n -t x1 -N 2 fio.dat | tr -d ' '); m2=$(od -A n -t x1 -j 196608 -N 2 fio.dat | tr -d ' ')
[ "$m" = caac ] && [ "$m2" = caac ] && ok "blocks start with the verify header magic 0xacca" || bad "header magic: $m / $m2"
# 2. corruption
printf 'X' | dd of=fio.dat bs=1 seek=200000 conv=notrunc 2>/dev/null
cat > rd.fio <<'EOJ'
[seqread]
filename=fio.dat
size=2m
bs=64k
ioengine=psync
verify=crc32c
rw=read
EOJ
r=$(to 120 "$F" rd.fio 2>&1); rc=$?
[ $rc -ne 0 ] && echo "$r" | has "crc32c: verify failed at file fio.dat offset 196608, length 65536" \
  && ok "a corrupted byte: crc32c verify failure at offset 196608 (exit $rc)" || { bad "corruption not detected (rc $rc)"; echo "$r"; }
# 3. pattern
cat > pat.fio <<'EOJ'
[pat]
filename=pat.dat
size=64k
bs=4k
ioengine=psync
rw=write
verify=pattern
verify_pattern=0x5aa5c33c
do_verify=1
EOJ
r=$(to 120 "$F" pat.fio 2>&1); rc=$?
u=$(od -A n -v -t x1 pat.dat | awk '{for (i=1;i<=NF;i++) {k=(n++)%4; c[k" "$i]++}} END {for (x in c) print x, c[x]}' | sort | tr '\n' ' ')
[ $rc -eq 0 ] && [ "$u" = "0 5a 16384 1 a5 16384 2 c3 16384 3 3c 16384 " ] \
  && ok "verify=pattern: the 64 KiB file is 0x5aa5c33c repeated, verified" || { bad "pattern (rc $rc): $u"; echo "$r"; }
# 4. POSIX AIO and mmap
cat > aio.fio <<'EOJ'
[aio]
filename=aio.dat
size=1m
bs=4k
rw=randwrite
ioengine=posixaio
iodepth=8
verify=md5
do_verify=1
randseed=1234
[mmapread]
stonewall
filename=aio.dat
size=1m
bs=4k
rw=read
ioengine=mmap
verify=md5
EOJ
to 120 "$F" --output-format=json --output=aio.json aio.fio > aio.log 2>&1; rc=$?
r=$(awk -F' : ' '/"jobname"/ {gsub(/[",]/, "", $2); j=$2; n=0}
                  /"error"/ {gsub(/,/, "", $2); print j, "error", $2}
                  /"io_kbytes"/ {gsub(/,/, "", $2); n++; print j, n, $2}' aio.json | tr '\n' ' ')
[ $rc -eq 0 ] && [ "$r" = "aio error 0 aio 1 1024 aio 2 1024 aio 3 0 mmapread error 0 mmapread 1 1024 mmapread 2 0 mmapread 3 0 " ] \
  && ok "posixaio random writes (iodepth 8) and mmap reads, md5-verified: 1024 KiB each" || { bad "aio/mmap (rc $rc): $r"; cat aio.log; }
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
