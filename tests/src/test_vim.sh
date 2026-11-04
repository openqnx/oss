#!/bin/sh
# Vim: headless ex mode (vim -es) edits a file -- substitute, delete, append
# and sort -- and evaluates Vim script (lists, map(), dicts, functions); xxd
# makes and reverses a hex dump. Results compared with known ones.
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
N=vim
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
V=$L/bin/vim
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
HOME=$T; TERM=dumb; export HOME TERM
info "$("$V" --version 2>&1 | head -n 1)"
printf 'zeta foo\nalpha foo bar\ndelete me\nmid foo\n' > f.txt
"$V" -es -u NONE -i NONE -N -c '%s/foo/FOO/g' -c 'g/delete me/d' -c '$s/$/\rappended/' -c 'sort' -c 'wq' f.txt < /dev/null > log 2>&1
exp='alpha FOO bar
appended
mid FOO
zeta FOO'
[ "$(cat f.txt)" = "$exp" ] && ok "vim -es: substitute, :g delete, append, sort, write" || { bad "vim -es edit"; cat f.txt; head -3 log; }
cat > t.vim <<'EOF'
function! Fact(n)
  return a:n <= 1 ? 1 : a:n * Fact(a:n - 1)
endfunction
let sq = map(range(1, 5), 'v:val * v:val')
let d = {'os': 'QNX', 'ver': 8}
let out = [string(sq), 'fact10=' . Fact(10), d.os . '-' . d.ver, toupper('vim'), join(split('a:b:c', ':'), '+'), printf('%05.2f', 3.14159)]
call writefile(out, 'out.txt')
qall!
EOF
"$V" -es -u NONE -i NONE -N -S t.vim < /dev/null > log2 2>&1
exp='[1, 4, 9, 16, 25]
fact10=3628800
QNX-8
VIM
a+b+c
03.14'
[ "$(cat out.txt 2>/dev/null)" = "$exp" ] && ok "Vim script: map/range, recursion, dict, printf, writefile" || { bad "Vim script"; cat out.txt 2>/dev/null; head -3 log2; }
printf 'QNX\001\377' > bin.dat
h=$("$L/bin/xxd" -p bin.dat 2>&1)
[ "$h" = 514e5801ff ] && ok "xxd -p: 514e5801ff" || bad "xxd -p: $h"
echo "$h" | "$L/bin/xxd" -r -p > back.dat && cmp -s back.dat bin.dat && ok "xxd -r -p restores the bytes" || bad "xxd -r"
r=$("$L/bin/xxd" -l 3 bin.dat 2>&1)
case "$r" in "00000000: 514e 58"*"QNX"*) ok "xxd dump line" ;; *) bad "xxd dump: $r" ;; esac
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
