#!/bin/bash
# Build one group of run-level tests, for checking them while writing:
#   tests/run-level/build-group.sh <group> [x86_64|aarch64le]...
# Configures tests/build/<arch>-<group> with only that group, builds its
# port_test() targets, and checks its shell tests with sh -n. Output:
# tests/build/<arch>-<group>/bin/. (The QNX compiler needs to lock
# ~/.qnx/license: run it where that is writable.)
set -o pipefail
WS="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export QNX_SDP_ROOT="${QNX_SDP_ROOT:-$HOME/qnx805}"
G=$1; shift
F="$WS/tests/run-level/$G.cmake"
[ -f "$F" ] || { echo "no $F" >&2; exit 1; }
targets=$(sed -n 's/^[[:space:]]*port_test(\([A-Za-z0-9_]*\).*/\1/p' "$F")
scripts=$(sed -n 's/^[[:space:]]*port_script(\([A-Za-z0-9_]*\).*/\1/p' "$F")
rc=0
for s in $scripts; do
  sh -n "$WS/tests/src/$s.sh" || { echo "!! $s.sh: syntax"; rc=1; }
done
for arch in "${@:-x86_64 aarch64le}"; do
  for a in $arch; do
    ( source "$WS/env.sh" "$a" >/dev/null || exit 1
      B="$WS/tests/build/$a-$G"
      cmake -S "$WS/tests" -B "$B" -G Ninja -DTEST_GROUPS="$G" \
            -DCMAKE_TOOLCHAIN_FILE="$WS/toolchain/qnx-$a.cmake" -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_INSTALL_PREFIX="$PREFIX" > "$B.log" 2>&1 || { echo "!! [$a] configure failed: $B.log"; exit 1; }
      [ -z "$targets" ] || cmake --build "$B" -j"$(nproc)" --target $targets -- -k 0 >> "$B.log" 2>&1 \
        || { echo "!! [$a] build failed: $B.log"; grep -E "error|undefined" "$B.log" | head -20; exit 1; }
      echo "== [$a] $G: $(echo $targets | wc -w) programs built, $(echo $scripts | wc -w) scripts" ) || rc=1
  done
done
exit $rc
