#!/bin/sh
# rsync: copy a tree locally (-a), then an incremental run that transfers
# only the changed file, --delete, --exclude, --checksum and a dry run with
# itemized changes; then an rsync daemon on loopback serving a module that a
# client lists and pulls from (rsync:// protocol).
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
N=rsync
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
RS=$L/bin/rsync
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
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
info "$("$RS" --version 2>&1 | head -n 1)"
mkdir -p src/sub/deeper src/empty
echo "alpha" > src/a.txt; echo "beta" > src/sub/b.txt; echo "gamma" > src/sub/deeper/c.txt
i=0; while [ $i -lt 3000 ]; do echo "bulk line $i"; i=$((i+1)); done > src/big.txt
echo junk > src/junk.tmp; ln -s a.txt src/link
"$RS" -a src/ dst/ && diff -r src dst >/dev/null && ok "-a copy: tree identical (diff -r)" || bad "-a copy"
[ -L dst/link ] && ok "symlink preserved" || bad "symlink"
sleep 1; echo "alpha changed" > src/a.txt
r=$("$RS" -a --out-format='%n' src/ dst/ 2>&1)
[ "$r" = a.txt ] && ok "incremental: only a.txt transferred" || bad "incremental transferred: $(echo $r)"
cmp -s src/a.txt dst/a.txt && ok "a.txt updated" || bad "a.txt not updated"
rm src/sub/b.txt
r=$("$RS" -an --delete -i src/ dst/ 2>&1)
case "$r" in *"*deleting"*"sub/b.txt"*) ok "dry run itemizes the deletion" ;; *) bad "dry run: $r" ;; esac
[ -f dst/sub/b.txt ] && ok "dry run changed nothing" || bad "dry run deleted"
"$RS" -a --delete --exclude='*.tmp' src/ dst/ && [ ! -f dst/sub/b.txt ] && [ -f dst/junk.tmp ] && ok "--delete removed b.txt, --exclude kept junk.tmp untouched" || bad "--delete/--exclude"
"$RS" -a --exclude='*.tmp' src/ ex/ && [ ! -f ex/junk.tmp ] && [ -f ex/big.txt ] && ok "--exclude='*.tmp' on a fresh copy" || bad "--exclude fresh copy"
echo "alpha CHANGED" > dst/a.txt; touch -r src/a.txt dst/a.txt 2>/dev/null
r=$("$RS" -ac --out-format='%n' src/ dst/ 2>&1)
[ "$r" = a.txt ] && cmp -s src/a.txt dst/a.txt && ok "--checksum finds same-size same-mtime change" || bad "--checksum: $(echo $r)"
unset RSYNC_PROXY
P=$((20000 + $$ % 5000))
mkdir mod
cat > rsyncd.conf <<EOF
pid file = $T/rsyncd.pid
use chroot = no
$([ "$(id -u)" = 0 ] && printf 'uid = 0\ngid = 0\n')
address = 127.0.0.1
port = $P
[qnx]
  path = $T/src
  read only = yes
  comment = QNX test module
EOF
"$RS" --daemon --no-detach --config="$T/rsyncd.conf" > d.log 2>&1 &
D=$!; track $D
i=0; while [ $i -lt 20 ]; do r=$(to 20 "$RS" "rsync://127.0.0.1:$P/" 2>&1) && break; sleep 1; i=$((i+1)); done
case "$r" in *qnx*"QNX test module"*) ok "daemon on 127.0.0.1:$P lists module qnx" ;; *) bad "daemon module list: $r"; head -5 d.log ;; esac
to 60 "$RS" -a "rsync://127.0.0.1:$P/qnx/" pulled/ 2> pull.err && cmp -s pulled/big.txt src/big.txt && [ -f pulled/sub/deeper/c.txt ] \
  && ok "pull over rsync:// (big.txt identical)" || { bad "pull over rsync://"; head -3 pull.err; }
kill $D 2>/dev/null; wait $D 2>/dev/null
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
