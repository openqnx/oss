#!/bin/sh
# glslang: glslangValidator compiles a known GLSL 450 fragment shader (a
# uniform block) and a vertex shader to SPIR-V; spirv-val validates them and
# spirv-dis shows the entry points, the binding and the member names; an
# undeclared identifier is an error; -Os goes through SPIRV-Tools; GLSL ES.
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
N=glslang
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
info "$("$B/glslangValidator" --version 2>&1 | head -n 1)"
cat > s.frag <<'EOS'
#version 450
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 color;
layout(set = 0, binding = 0) uniform Params { vec4 tint; float gain; } p;
void main() { color = vec4(uv, 0.5, 1.0) * p.tint * p.gain; }
EOS
cat > s.vert <<'EOS'
#version 450
layout(location = 0) in vec3 pos;
layout(location = 0) out vec2 uv;
void main() { uv = pos.xy * 0.5 + 0.5; gl_Position = vec4(pos, 1.0); }
EOS
to 60 "$B/glslangValidator" -V s.frag -o frag.spv > out.txt 2>&1 && ok "glslangValidator -V compiles the fragment shader to SPIR-V" || { bad "glslangValidator frag"; cat out.txt; }
to 60 "$B/glslangValidator" -V s.vert -o vert.spv > out.txt 2>&1 && ok "... and the vertex shader" || { bad "vert"; cat out.txt; }
[ "$(od -An -tx4 -N4 frag.spv | tr -d ' ')" = 07230203 ] && ok "SPIR-V magic" || bad "magic"
to 30 "$B/spirv-val" frag.spv && to 30 "$B/spirv-val" vert.spv && ok "spirv-val: both valid" || bad "spirv-val"
"$B/spirv-dis" frag.spv > f.dis 2>&1
grep -q 'OpEntryPoint Fragment %main "main"' f.dis && grep -q 'OpDecorate %p Binding 0' f.dis && grep -q 'OpMemberName %Params 1 "gain"' f.dis \
  && ok "spirv-dis: fragment entry point, binding 0, member 'gain'" || { bad "disassembly"; head -40 f.dis; }
"$B/spirv-dis" vert.spv | grep -q 'OpEntryPoint Vertex %main "main"' && ok "vertex entry point" || bad "vertex entry point"
printf '#version 450\nvoid main() { undefined_var = 1; }\n' > bad.frag
to 30 "$B/glslangValidator" -V bad.frag -o bad.spv > out.txt 2>&1; rc=$?
[ $rc -ne 0 ] && grep -q "undeclared identifier" out.txt && ok "an undeclared identifier is an error" || { bad "bad shader: rc $rc"; cat out.txt; }
to 30 "$B/glslangValidator" s.frag > out.txt 2>&1 && ok "OpenGL GLSL validation (no -V)" || { bad "validation"; cat out.txt; }
to 30 "$B/glslangValidator" -V --target-env vulkan1.3 -Os s.frag -o o.spv > out.txt 2>&1 && "$B/spirv-val" --target-env vulkan1.3 o.spv \
  && ok "-Os for Vulkan 1.3 through SPIRV-Tools' optimizer" || { bad "-Os"; cat out.txt; }
printf '#version 310 es\nprecision mediump float;\nlayout(location=0) out vec4 c;\nvoid main(){ c = vec4(1.0); }\n' > es.frag
to 30 "$B/glslangValidator" -V es.frag -o es.spv > out.txt 2>&1 && ok "GLSL ES 3.10" || { bad "ES"; cat out.txt; }
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
