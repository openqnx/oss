#!/bin/sh
# BusyBox: its applets doing real work -- ash runs a script with functions
# and arithmetic, sha256sum/md5sum of known strings, sort|uniq -c, sed, tr,
# factor, expr, seq, a tar+gzip round trip, xz/bzip2 decompression of what
# its own compressors made, diff of two files, and find.
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
N=busybox
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
BB=$L/bin/busybox
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
info "$("$BB" 2>&1 | head -n 1)"
n=$("$BB" --list 2>/dev/null | wc -l | tr -d ' ')
[ "${n:-0}" -gt 50 ] && ok "--list: $n applets" || bad "--list: ${n:-0} applets"
eq() { [ "$2" = "$3" ] && ok "$1" || bad "$1: got '$2', want '$3'"; }
cat > s.sh <<'EOF'
sq() { echo $(( $1 * $1 )); }
t=0; for i in 1 2 3 4 5; do t=$(( t + $(sq $i) )); done
case "busybox-ash" in *-ash) kind=ash ;; *) kind=other ;; esac
echo "sum=$t kind=$kind len=${#kind} strip=${kind#a}"
EOF
eq "ash: script (function, loop, case, expansions)" "$("$BB" sh s.sh 2>&1)" "sum=55 kind=ash len=3 strip=sh"
eq "sha256sum 'abc'" "$(printf abc | "$BB" sha256sum | cut -d' ' -f1)" ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
eq "md5sum 'abc'" "$(printf abc | "$BB" md5sum | cut -d' ' -f1)" 900150983cd24fb0d6963f7d28e17f72
eq "sort | uniq -c" "$(printf 'b\na\nb\nc\nb\na\n' | "$BB" sort | "$BB" uniq -c | "$BB" tr -s ' ' | "$BB" tr '\n' ';')" " 2 a; 3 b; 1 c;"
eq "cut -d: -f2 and tac" "$(printf 'a:1\nb:2\nc:3\n' | "$BB" cut -d: -f2 | "$BB" tac | "$BB" tr '\n' ';')" "3;2;1;"
eq "factor 360" "$("$BB" factor 360)" "360: 2 2 2 3 3 5"
eq "expr 7 \\* 6" "$("$BB" expr 7 \* 6)" 42
eq "seq 1 3 10" "$("$BB" seq 1 3 10 | "$BB" tr '\n' ' ')" "1 4 7 10 "
mkdir -p src/d; echo hello > src/a.txt; "$BB" seq 1 500 > src/d/n.txt
"$BB" tar -czf t.tgz src && mkdir out && "$BB" tar -xzf t.tgz -C out && "$BB" diff -r src out/src >/dev/null \
  && ok "tar -czf / -xzf round trip (diff -r identical)" || bad "tar+gzip round trip"
eq "tar -tzf lists both files" "$("$BB" tar -tzf t.tgz | "$BB" grep -c 'txt$')" 2
# 'hello qnx' compressed by xz 5 on the build host (busybox's xz only decompresses)
printf '\375\067\172\130\132\000\000\004\346\326\264\106\002\000\041\001\026\000\000\000\164\057\345\243\001\000\011\150\145\154\154\157\040\161\156\170\012\000\000\000\173\162\013\045\325\263\223\221\000\001\042\012\025\032\341\147\037\266\363\175\001\000\000\000\000\004\131\132' > h.xz
eq "unxz of a known .xz" "$("$BB" unxz -c h.xz 2>&1)" "hello qnx"
"$BB" gzip -9 -c src/d/n.txt > n.gz && "$BB" gunzip -c n.gz | cmp -s - src/d/n.txt && ok "gzip -9 / gunzip round trip" || bad "gzip round trip"
"$BB" bzip2 -c src/d/n.txt > n.bz2 && "$BB" bunzip2 -c n.bz2 | cmp -s - src/d/n.txt && ok "bzip2 / bunzip2 round trip" || bad "bzip2 round trip"
printf 'a\nb\nc\n' > x1; printf 'a\nB\nc\n' > x2
d=$("$BB" diff -u x1 x2 | "$BB" grep '^[-+][^-+]' | "$BB" tr '\n' ' ')
eq "diff -u finds the changed line" "$d" "-b +B "
eq "find -name '*.txt' | wc -l" "$("$BB" find src -name '*.txt' | "$BB" wc -l | tr -d ' ')" 2
eq "grep -E -o" "$(echo 'qnx800 sdp8 x86_64' | "$BB" grep -Eo '[0-9]+' | "$BB" tr '\n' ',')" "800,8,86,64,"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
