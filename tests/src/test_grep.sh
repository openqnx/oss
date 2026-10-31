#!/bin/sh
# GNU grep: basic, extended and Perl (PCRE2) regular expressions, -c -o -i
# -w -v -n -l -r --include, context lines and a fixed-string search over a
# small known text -- each result compared with the known one.
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
N=grep
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
G=$L/bin/grep
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
info "$("$G" --version 2>&1 | head -n 1)"
cat > f.txt <<'EOF'
QNX Neutrino RTOS 8.0
the quick brown fox
The Quick Brown Dog
microkernel: procnto-smp-instr
ip 192.168.1.10 port 8080
ip 10.0.0.1 port 22
a.b literal dot
end
EOF
eq() { [ "$2" = "$3" ] && ok "$1" || bad "$1: got '$2', want '$3'"; }
j() { tr '\n' '|'; }
eq "basic regex ^ip" "$("$G" -c '^ip' f.txt)" 2
eq "-i case-insensitive" "$("$G" -ci 'quick' f.txt)" 2
eq "-w whole word" "$("$G" -cw 'port' f.txt)" 2
eq "-v invert" "$("$G" -vc 'o' f.txt)" 1
eq "-n line numbers" "$("$G" -n 'micro' f.txt)" "4:microkernel: procnto-smp-instr"
eq "-E -o IPv4 addresses" "$("$G" -Eo '([0-9]{1,3}\.){3}[0-9]{1,3}' f.txt | j)" "192.168.1.10|10.0.0.1|"
eq "-P lookbehind (PCRE2)" "$("$G" -Po '(?<=port )\d+' f.txt | j)" "8080|22|"
eq "-P lazy quantifier" "$("$G" -Po 'p.+?o' f.txt | head -n 1)" "pro"
eq "-F fixed string a.b" "$("$G" -F 'a.b' f.txt)" "a.b literal dot"
eq "-A1 context" "$("$G" -A1 'fox' f.txt | j)" "the quick brown fox|The Quick Brown Dog|"
mkdir -p tree/sub; echo "needle here" > tree/sub/x.c; echo "needle too" > tree/y.h; echo "needle" > tree/z.txt
eq "-r --include='*.[ch]' -l" "$("$G" -rl --include='*.[ch]' needle tree | sort | j)" "tree/sub/x.c|tree/y.h|"
"$G" -q 'absent-pattern' f.txt; eq "exit status 1 when nothing matches" "$?" 1
i=0; : > big.txt; while [ $i -lt 3000 ]; do echo "row $i value $((i*7))" >> big.txt; i=$((i+1)); done
eq "-c over 3000 lines (values ending in 77)" "$("$G" -Ec 'value [0-9]*77$' big.txt)" 30
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
