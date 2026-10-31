#!/bin/sh
# ninja: a build.ninja with two rules, a variable, an implicit dependency and
# a phony target -- builds, then reports no work, rebuilds only what an edit
# affects, and answers -t queries (targets, query, commands, clean).
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
N=ninja
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
NJ=$L/bin/ninja
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
v=$("$NJ" --version 2>&1); info "ninja $v"
echo one > a.txt; echo two > b.txt; echo "header" > hdr.txt
cat > build.ninja <<'EOF'
tag = QNX
rule up
  command = tr a-z A-Z < $in > $out
  description = UP $out
rule cat
  command = { echo "$tag"; cat $in; } > $out
build a.up: up a.txt
build b.up: up b.txt | hdr.txt
build all.txt: cat a.up b.up
build all: phony all.txt
default all
EOF
"$NJ" > log1 2>&1 && ok "first build" || { bad "first build"; cat log1; }
[ "$(cat all.txt 2>/dev/null | tr '\n' ' ')" = "QNX ONE TWO " ] && ok "all.txt = QNX ONE TWO" || bad "all.txt: $(cat all.txt 2>/dev/null)"
r=$("$NJ" 2>&1)
case "$r" in *"no work to do"*) ok "second run: no work to do" ;; *) bad "second run: $r" ;; esac
sleep 1; echo changed > hdr.txt
r=$("$NJ" -v 2>&1)
case "$r" in *"b.txt > b.up"*) ok "implicit dependency hdr.txt rebuilt b.up" ;; *) bad "implicit dep: $r" ;; esac
case "$r" in *"a.txt > a.up"*) bad "a.up rebuilt needlessly" ;; *) ok "a.up left alone" ;; esac
[ "$("$NJ" -t targets all 2>&1 | tr '\n' ';')" = "a.up: up;b.up: up;all.txt: cat;all: phony;" ] && ok "-t targets all" || bad "-t targets: $("$NJ" -t targets all 2>&1 | tr '\n' ';')"
q=$("$NJ" -t query all.txt 2>&1)
case "$q" in *"input: cat"*a.up*b.up*"outputs:"*all*) ok "-t query all.txt (inputs and outputs)" ;; *) bad "-t query: $q" ;; esac
c=$("$NJ" -t commands a.up 2>&1)
[ "$c" = "tr a-z A-Z < a.txt > a.up" ] && ok "-t commands a.up" || bad "-t commands: $c"
"$NJ" -t clean >/dev/null 2>&1 && [ ! -f all.txt ] && [ ! -f a.up ] && ok "-t clean removed outputs" || bad "-t clean"
"$NJ" -n > dry 2>&1 && [ ! -f all.txt ] && ok "-n dry run builds nothing" || bad "-n dry run"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
