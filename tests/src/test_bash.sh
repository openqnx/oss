#!/bin/sh
# bash: a script with indexed and associative arrays, functions with local
# variables and recursion, parameter expansion, arithmetic, here-strings,
# printf -v, mapfile and a pipeline -- its output compared with the known one.
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
N=bash
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
B=$L/bin/bash
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
v=$("$B" -c 'echo "$BASH_VERSION"' 2>&1) && ok "bash runs (BASH_VERSION $v)" || bad "bash -c: $v"
cat > s.bash <<'EOF'
fact() { local n=$1; (( n <= 1 )) && { echo 1; return; }; echo $(( n * $(fact $((n-1))) )); }
declare -a a=(cherry apple banana)
declare -A m=([one]=1 [two]=2 [three]=3)
sorted=$(printf '%s\n' "${a[@]}" | sort | tr '\n' ' ')
echo "len=${#a[@]} second=${a[1]} sorted=${sorted% }"
sum=0; for k in "${!m[@]}"; do (( sum += m[$k] )); done; echo "assoc-sum=$sum keys=${#m[@]}"
echo "fact10=$(fact 10)"
s="Hello, QNX world"; echo "upper=${s^^} sub=${s:7:3} repl=${s/world/bash} len=${#s}"
printf -v pv '%05d|%-4s|%x' 42 ab 255; echo "printf-v=$pv"
read -r x y <<< "left right"; echo "herestring=$y,$x"
mapfile -t lines <<< "$(printf 'l1\nl2\nl3')"; echo "mapfile=${#lines[@]}:${lines[2]}"
[[ "qnx-8.0" =~ ^([a-z]+)-([0-9]+)\.([0-9]+)$ ]] && echo "regex=${BASH_REMATCH[1]}/${BASH_REMATCH[2]}"
echo "brace=$(echo {1..5} | tr ' ' ,) arith=$(( (7**2) % 10 + 0x10 ))"
EOF
cat > exp.txt <<'EOF'
len=3 second=apple sorted=apple banana cherry
assoc-sum=6 keys=3
fact10=3628800
upper=HELLO, QNX WORLD sub=QNX repl=Hello, QNX bash len=16
printf-v=00042|ab  |ff
herestring=right,left
mapfile=3:l3
regex=qnx/8
brace=1,2,3,4,5 arith=25
EOF
"$B" s.bash > out.txt 2> err.txt
if cmp -s out.txt exp.txt; then ok "script output matches (arrays, assoc arrays, functions, expansions)"
else bad "script output differs"; diff exp.txt out.txt | head -20; head -5 err.txt; fi
r=$("$B" -c 'f() { return 3; }; f; echo $?; set -o pipefail; false | true; echo $?' 2>&1)
[ "$(echo $r)" = "3 1" ] && ok "return status and pipefail" || bad "status/pipefail: $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
