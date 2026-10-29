#!/bin/bash
# Cross-build the port test suite for one or both architectures.
#   ./tests/build-tests.sh [aarch64le|x86_64]
set -o pipefail
WS="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ARCHES="${1:-aarch64le x86_64}"
rc=0
for arch in $ARCHES; do
  source "$WS/env.sh" "$arch" >/dev/null || exit 1
  B="$WS/tests/build/$arch"
  echo "== building port tests [$arch]"
  rm -rf "$B"
  cmake -S "$WS/tests" -B "$B" -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$WS/toolchain/qnx-$arch.cmake" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        >"$WS/logs/tests.$arch.log" 2>&1 \
    && cmake --build "$B" -j"$(nproc)" -- -k 0 >>"$WS/logs/tests.$arch.log" 2>&1 \
    && DESTDIR="$STAGE" cmake --install "$B" >>"$WS/logs/tests.$arch.log" 2>&1
  if [ $? -eq 0 ]; then
    echo "== port tests [$arch] OK -> $STAGE$PREFIX/bin/qnx-port-tests/ ($(ls "$B/bin" | grep -c '^test_') binaries)"
  else
    echo "!! port tests [$arch] FAILED -- see $WS/logs/tests.$arch.log"; rc=1
  fi
done
exit $rc
