#!/bin/sh
# Godot 4 export templates (the QNX runtime that plays exported games), no
# display: --version, and --headless running a tiny project from a text
# project.godot -- a GDScript SceneTree script computes known values (array
# sort, string, Dictionary, math, a JSON round trip), prints them and quits.
# Both the release and the debug template are exercised.
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
N=godot
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
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
HOME=$T; XDG_DATA_HOME=$T/data; XDG_CONFIG_HOME=$T/config; XDG_CACHE_HOME=$T/cache; export HOME XDG_DATA_HOME XDG_CONFIG_HOME XDG_CACHE_HOME
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
mkdir proj
cat > proj/project.godot <<'EOF'
config_version=5

[application]

config/name="QNX Godot Test"
EOF
cat > proj/qnx_test.gd <<'EOF'
extends SceneTree

func _init():
	var a = [5, 3, 9, 1]
	a.sort()
	var d = {"os": "QNX", "ver": 8}
	var j = JSON.parse_string(JSON.stringify({"k": [1, 2, 3]}))
	print("GD-RESULT sorted=%s upper=%s dict=%s-%d pow=%d sqrt=%.3f json=%d" % [str(a), "neutrino".to_upper(), d["os"], d["ver"], int(pow(2, 10)), sqrt(2.0), j["k"].size()])
	quit(0)
EOF
want='GD-RESULT sorted=[1, 3, 5, 9] upper=NEUTRINO dict=QNX-8 pow=1024 sqrt=1.414 json=3'
for kind in release debug; do
  G=$L/bin/godot-template-$kind-qnxscreen
  [ -x "$G" ] || { bad "$kind template not installed"; continue; }
  v=$(to 60 "$G" --version 2>&1 | tail -n 1); case "$v" in 4.*) ok "$kind --version: $v" ;; *) bad "$kind --version: $v" ;; esac
  to 100 "$G" --headless --path "$T/proj" --script qnx_test.gd > $kind.log 2>&1; rc=$?
  r=$(awk '/^GD-RESULT/ {print; exit}' $kind.log)
  [ "$r" = "$want" ] && ok "$kind --headless GDScript: $r" || { bad "$kind GDScript (rc=$rc): '$r'"; tail -n 6 $kind.log; }
  [ $rc -eq 0 ] && ok "$kind: quit(0) exit status" || bad "$kind exit status $rc"
done
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
