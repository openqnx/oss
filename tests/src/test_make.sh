#!/bin/sh
# GNU make: a Makefile with a pattern rule, automatic variables, text
# functions ($(patsubst), $(foreach), $(shell), $(call)), an order of
# prerequisites and parallel jobs; then an up-to-date check (nothing rebuilt)
# and an incremental rebuild after one input changes.
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
N=make
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
MK=$L/bin/make
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
info "$("$MK" --version 2>&1 | head -n 1)"
for n in alpha beta gamma; do echo "source $n" > $n.in; done
cat > Makefile <<'EOF'
SRCS := $(sort $(wildcard *.in))
OUTS := $(patsubst %.in,%.out,$(SRCS))
upper = $(shell echo $(1) | tr a-z A-Z)
all: total.txt
%.out: %.in
	@echo "built $@ from $<"
	@sed 's/source/OUT/' $< > $@
total.txt: $(OUTS)
	@cat $^ > $@
	@echo "$(words $^) parts: $(foreach o,$^,$(basename $(o)))" >> $@
	@echo "upper: $(call upper,qnx make)" >> $@
.PHONY: all clean
clean:
	rm -f *.out total.txt
EOF
"$MK" -j3 > log1 2>&1; rc=$?
[ $rc -eq 0 ] && ok "make -j3 succeeded" || { bad "make -j3 (rc=$rc)"; head -5 log1; }
exp='OUT alpha
OUT beta
OUT gamma
3 parts: alpha beta gamma
upper: QNX MAKE'
[ "$(cat total.txt 2>/dev/null)" = "$exp" ] && ok "total.txt content (pattern rule, \$^, foreach, call, shell)" || { bad "total.txt content"; cat total.txt; }
[ "$(sort log1 | tr '\n' ';')" = "built alpha.out from alpha.in;built beta.out from beta.in;built gamma.out from gamma.in;" ] \
  && ok "three pattern-rule builds" || { bad "build log"; cat log1; }
r=$("$MK" 2>&1)
case "$r" in *"Nothing to be done"*|*"is up to date"*) ok "second run: up to date" ;; *) bad "second run rebuilt: $r" ;; esac
sleep 1; echo "source beta2" > beta.in
r=$("$MK" 2>&1)
[ "$r" = "built beta.out from beta.in" ] && ok "incremental: only beta.out rebuilt" || bad "incremental: '$r'"
[ "$(sed -n 2p total.txt)" = "OUT beta2" ] && ok "total.txt updated" || bad "total.txt not updated"
r=$("$MK" -n clean 2>&1)
[ "$r" = "rm -f *.out total.txt" ] && ok "make -n prints without running" || bad "make -n: $r"
[ -f total.txt ] || bad "make -n removed files"
r=$("$MK" -s -f - <<'EOF'
X = 3
Y = $(shell expr $(X) \* 14)
all: ; @echo $(if $(filter 42,$(Y)),forty-two,other) $(strip $(subst a,o,banana))
EOF
)
[ "$r" = "forty-two bonono" ] && ok "makefile from stdin (if/filter/subst)" || bad "stdin makefile: $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
