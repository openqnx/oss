#!/bin/sh
# spirv-tools: assemble a known fragment shader with spirv-as (the SPIR-V
# magic), validate it for Vulkan, disassemble it back (entry point, constant,
# the OpName), reject a broken module with spirv-val, optimise and strip it
# with spirv-opt, spirv-cfg and spirv-link.
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
N=spirv_tools
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
info "$("$B/spirv-as" --version 2>&1 | head -n 1)"
# a minimal fragment shader writing a constant colour, in SPIR-V assembly
cat > f.spvasm <<'EOS'
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %color
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
               OpDecorate %color Location 0
       %void = OpTypeVoid
     %fnvoid = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output = OpTypePointer Output %v4float
      %color = OpVariable %_ptr_Output Output
    %float_1 = OpConstant %float 1
  %float_0_5 = OpConstant %float 0.5
     %result = OpConstantComposite %v4float %float_1 %float_0_5 %float_0_5 %float_1
       %main = OpFunction %void None %fnvoid
      %entry = OpLabel
               OpStore %color %result
               OpReturn
               OpFunctionEnd
EOS
to 30 "$B/spirv-as" --target-env vulkan1.0 f.spvasm -o f.spv 2> err.txt && ok "spirv-as assembles the shader" || { bad "spirv-as"; cat err.txt; }
magic=$(od -An -tx4 -N4 f.spv 2>/dev/null | tr -d ' ')
[ "$magic" = 07230203 ] && ok "the binary starts with the SPIR-V magic 0x07230203" || bad "magic '$magic'"
to 30 "$B/spirv-val" --target-env vulkan1.0 f.spv 2> err.txt && ok "spirv-val: valid for Vulkan 1.0" || { bad "spirv-val"; cat err.txt; }
to 30 "$B/spirv-dis" --raw-id f.spv -o f.dis 2> err.txt
grep -q 'OpEntryPoint Fragment' f.dis && grep -q 'OpConstant %[0-9]* 0.5' f.dis && ok "spirv-dis: entry point and the 0.5 constant back" || { bad "spirv-dis"; cat f.dis err.txt; }
"$B/spirv-dis" f.spv | grep -q '%main = OpFunction' && ok "spirv-dis names %main from its OpName" || bad "names"
# break it: OpStore of a float into a vec4 output
sed 's/OpStore %color %result/OpStore %color %float_1/' f.spvasm > bad.spvasm
"$B/spirv-as" bad.spvasm -o bad.spv 2>/dev/null
to 30 "$B/spirv-val" bad.spv > out.txt 2>&1; rc=$?
[ $rc -ne 0 ] && grep -qi 'OpStore' out.txt && ok "spirv-val rejects a type-mismatched OpStore" || { bad "spirv-val on the broken module: rc $rc"; cat out.txt; }
to 60 "$B/spirv-opt" -O f.spv -o o.spv 2> err.txt && "$B/spirv-val" o.spv 2>/dev/null \
  && ok "spirv-opt -O: still valid ($(wc -c < f.spv | tr -d ' ') -> $(wc -c < o.spv | tr -d ' ') bytes)" || { bad "spirv-opt"; cat err.txt; }
to 60 "$B/spirv-opt" --strip-debug f.spv -o s.spv && ! "$B/spirv-dis" s.spv | grep -q OpName && ok "spirv-opt --strip-debug removes OpName" || bad "strip-debug"
"$B/spirv-cfg" f.spv 2>/dev/null | grep -q 'digraph' && ok "spirv-cfg: a dot graph of the CFG" || bad "spirv-cfg"
"$B/spirv-link" f.spv -o l.spv 2>/dev/null && "$B/spirv-val" l.spv && ok "spirv-link of one module validates" || bad "spirv-link"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
