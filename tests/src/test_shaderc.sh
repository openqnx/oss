#!/bin/sh
# shaderc: glslc compiles a GLSL fragment shader with a -D macro to valid
# SPIR-V carrying the constant, preprocesses (-E), emits assembly (-S),
# reports an undefined macro, compiles HLSL, optimises (-O), lists
# dependencies (-M).
# Runs in the background with its output to a log, for at most 170 s (a hang
# ends as a FAIL with what it printed so far).
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
N=shaderc
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
D=$(cd "$(dirname "$0")" && pwd)   # helpers next to this test
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
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
B=$L/bin
info "$("$B/glslc" --version 2>&1 | head -n 1)"
cat > s.frag <<'EOS'
#version 450
layout(location = 0) out vec4 color;
layout(push_constant) uniform PC { float k; } pc;
void main() { color = vec4(SCALE * pc.k); }
EOS
to 60 "$B/glslc" -DSCALE=0.25 s.frag -o s.spv 2> err.txt && ok "glslc compiles a fragment shader (stage from the extension, -D macro)" || { bad "glslc"; cat err.txt; }
[ "$(od -An -tx4 -N4 s.spv 2>/dev/null | tr -d ' ')" = 07230203 ] && ok "SPIR-V magic" || bad "magic"
"$B/spirv-val" s.spv 2>/dev/null && ok "spirv-val: valid" || bad "spirv-val"
"$B/spirv-dis" s.spv 2>/dev/null | grep -q 'OpConstant %float 0.25' && ok "the -DSCALE=0.25 constant is in the module" || bad "constant 0.25"
to 30 "$B/glslc" -E -DSCALE=3 s.frag 2>/dev/null | grep -q 'vec4(3 \* pc.k)' && ok "glslc -E preprocesses" || bad "-E"
to 30 "$B/glslc" -S s.frag -DSCALE=1.0 -o - 2>/dev/null | grep -q 'OpEntryPoint Fragment' && ok "glslc -S emits SPIR-V assembly" || bad "-S"
to 30 "$B/glslc" s.frag -o x.spv > err.txt 2>&1; rc=$?
[ $rc -ne 0 ] && grep -q "SCALE" err.txt && ok "without -DSCALE: an error naming SCALE" || { bad "error: rc $rc"; cat err.txt; }
printf 'float4 main(float4 p : SV_Position) : SV_Target { return p * 0.5; }\n' > h.hlsl
to 30 "$B/glslc" -x hlsl -fshader-stage=frag -fentry-point=main h.hlsl -o h.spv 2> err.txt && "$B/spirv-val" h.spv \
  && ok "HLSL front end" || { bad "HLSL"; cat err.txt; }
to 30 "$B/glslc" -O --target-env=vulkan1.2 -DSCALE=2.0 s.frag -o o.spv && "$B/spirv-val" --target-env vulkan1.2 o.spv && ok "-O for Vulkan 1.2" || bad "-O"
"$B/glslc" -M -DSCALE=1.0 s.frag 2>/dev/null | grep -q 's.frag' && ok "glslc -M: dependency output" || bad "-M"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
