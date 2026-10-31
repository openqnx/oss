#!/bin/sh
# tftp-hpa on loopback: in.tftpd serves a directory (-L foreground, -s
# chroot, -c create); the tftp client downloads a text file and a binary
# file (compared byte for byte), uploads a new file, and gets a clean error
# for a file that does not exist.
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
N=tftp-hpa
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
TD=$L/sbin/in.tftpd; TC=$L/bin/tftp
T=$(mktemp -d /tmp/qnxport-tftp.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
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
info "$("$TC" -V 2>&1 | head -n 1)"
mkdir srv; echo "hello over TFTP from QNX" > srv/hello.txt
i=0; while [ $i -lt 4000 ]; do printf 'block %05d of binary-ish data\n' $i; i=$((i+1)); done > srv/big.bin
printf '\000\001\002\377\376' >> srv/big.bin
chmod 755 srv; chmod 644 srv/*
P=$((16900 + $$ % 2000))
u=; [ "$(id -u)" = 0 ] && u="-u root"
"$TD" -L -a 127.0.0.1:$P -s "$T/srv" -c -p $u -v > d.log 2>&1 &
D=$!; track $D
sleep 2
kill -0 $D 2>/dev/null && ok "in.tftpd listening on 127.0.0.1:$P" || { bad "in.tftpd exited"; cat d.log; }
tc() { to 60 "$TC" -m binary 127.0.0.1 $P -c "$@" 2>&1; }
r=$(tc get hello.txt got.txt); cmp -s got.txt srv/hello.txt && ok "get hello.txt" || bad "get hello.txt: $r"
r=$(tc get big.bin big.bin); cmp -s big.bin srv/big.bin && ok "get big.bin ($(wc -c < big.bin | tr -d ' ') bytes, binary, multi-block)" || bad "get big.bin: $r"
echo "uploaded by the tftp client" > up.txt
r=$(tc put up.txt up.txt); cmp -s up.txt srv/up.txt && ok "put up.txt (created on the server)" || bad "put: $r"
r=$(tc get no-such-file nf.txt)
case "$r" in *"not found"*|*"File not found"*|*"Error code 1"*) ok "missing file: server error reported ($(echo $r | cut -c1-40))" ;; *) bad "missing file: $r" ;; esac
kill $D 2>/dev/null; wait $D 2>/dev/null
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
