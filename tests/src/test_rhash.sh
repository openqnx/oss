#!/bin/sh
# rhash (the program): known digests (standard test vectors, as Python's
# hashlib/zlib compute them) of "abc" and of the "quick brown fox" sentence
# through --printf formats; a SHA-256 sum file written with -o and checked
# with -c (OK), then the same check after a file changed (an error);
# --sfv output; the sfv-hash link name selects SFV mode. test_rhash_lib
# checks LibRHash through its C API.
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
N=rhash
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
RH=$L/bin/rhash
r=$("$RH" --version 2>&1); [ "$r" = "RHash v1.4.5" ] && ok "rhash --version: v1.4.5" || bad "version: $r"
printf 'abc' > abc.txt; printf 'The quick brown fox jumps over the lazy dog' > fox.txt
r=$("$RH" --printf='%{crc32} %{md5} %{sha1}\n' abc.txt 2>&1)
[ "$r" = "352441c2 900150983cd24fb0d6963f7d28e17f72 a9993e364706816aba3e25717850c26c9cd0d89d" ] && ok "abc: CRC32, MD5, SHA-1" || bad "abc: $r"
r=$("$RH" --printf='%{sha-256} %{sha3-256}\n' abc.txt 2>&1)
[ "$r" = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad 3a985da74fe225b2045c172d6bd390bd855f086e3e9d525b46bfe24511431532" ] && ok "abc: SHA-256, SHA3-256" || bad "abc sha2/3: $r"
r=$("$RH" --printf='%{crc32} %{md5} %{sha-256}\n' fox.txt 2>&1)
[ "$r" = "414fa339 9e107d9d372bb6826bd81d3542a419d6 d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592" ] && ok "quick brown fox: CRC32, MD5, SHA-256" || bad "fox: $r"
"$RH" --sha256 -o sums.sha256 abc.txt fox.txt 2>err.txt; rc=$?
r=$("$RH" -c sums.sha256 2>&1); rc2=$?
[ $rc -eq 0 ] && [ $rc2 -eq 0 ] && grep -q '^ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad  abc.txt$' sums.sha256 \
  && ok "-o sums.sha256 (sha256sum format), -c: both files OK" || bad "sum file (rc $rc/$rc2): $(cat err.txt) $r"
printf 'abd' > abc.txt
r=$("$RH" -c sums.sha256 2>&1); rc=$?
[ $rc -ne 0 ] && echo "$r" | has "abc.txt" && echo "$r" | grep -q -i 'err' && ok "-c after abc.txt changed: reported, exit $rc" || bad "-c changed (rc $rc): $r"
printf 'abc' > abc.txt
r=$("$RH" --sfv abc.txt 2>&1 | grep -v '^;'); [ "$r" = "abc.txt 352441C2" ] && ok "--sfv: abc.txt 352441C2" || bad "sfv: $r"
r=$("$L/bin/sfv-hash" abc.txt 2>&1 | grep -v '^;'); [ "$r" = "abc.txt 352441C2" ] && ok "the sfv-hash link: SFV mode" || bad "sfv-hash: $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
