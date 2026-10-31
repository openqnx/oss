#!/bin/sh
# zsh: a script with 1-based arrays, associative arrays, parameter-expansion
# flags, floating-point arithmetic, functions, glob qualifiers and printf --
# its output compared with the known one.
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
N=zsh
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
Z=$L/bin/zsh
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
v=$("$Z" -f -c 'echo $ZSH_VERSION' 2>&1) && ok "zsh runs (ZSH_VERSION $v)" || bad "zsh -c: $v"
mkdir g; : > g/a.txt; : > g/b.txt; : > g/c.log; mkdir g/sub
cat > s.zsh <<'EOF'
fib() {
  if (( $1 < 2 )); then echo $1; return; fi
  echo $(( $(fib $(( $1 - 1 ))) + $(fib $(( $1 - 2 ))) ))
}
a=(delta alpha charlie bravo)
echo "first=$a[1] last=$a[-1] n=$#a sorted=${(j:,:)${(@o)a}}"
typeset -A h; h=(red 1 green 2 blue 3)
echo "keys=${(j:,:)${(@ok)h}} green=$h[green]"
s="QNX Neutrino"; echo "upper=${(U)s} lower=${(L)s} words=${#${(z)s}}"
echo "split=${(j:+:)${(s:-:)${:-a-b-c}}}"
echo "fib10=$(fib 10)"
typeset -F 3 f; (( f = 22.0 / 7 )); echo "float=$f"
t=(g/*.txt(.:t)); d=(g/*(/:t)); echo "files=$t dirs=$d"
printf '%s=%04d\n' pad 7
echo "repeat=${(l:5::*:)${:-x}}"
EOF
cat > exp.txt <<'EOF'
first=delta last=bravo n=4 sorted=alpha,bravo,charlie,delta
keys=blue,green,red green=2
upper=QNX NEUTRINO lower=qnx neutrino words=2
split=a+b+c
fib10=55
float=3.143
files=a.txt b.txt dirs=sub
pad=0007
repeat=****x
EOF
"$Z" -f s.zsh > out.txt 2> err.txt
if cmp -s out.txt exp.txt; then ok "script output matches (arrays, assoc arrays, flags, floats, globs)"
else bad "script output differs"; diff exp.txt out.txt | head -20; head -5 err.txt; fi
r=$("$Z" -f -c 'setopt extendedglob; x=(g/^*.log(N:t)); print $#x' 2>&1)
[ "$r" = 3 ] && ok "extended glob negation (^*.log)" || bad "extendedglob: $r"
r=$("$Z" -f -c 'autoload -U zmv 2>/dev/null; zmodload zsh/datetime 2>/dev/null && print -r -- ${EPOCHSECONDS:+datetime}' 2>&1)
info "zsh/datetime module: ${r:-not available}"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
