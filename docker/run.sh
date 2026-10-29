#!/bin/bash
# Run a command (default: a shell) in the openqnx/oss build container.
#
#   QNX_SDP_ROOT=$HOME/qnx800 docker/run.sh [command...]
#
# Mounted from the host:
#   this repository        -> /work            (read-write: src/, build/, stage/)
#   $QNX_SDP_ROOT          -> /opt/qnx800      (read-only)
#   ~/.qnx                 -> ~/.qnx           (read-only; the QNX license)
#
# Nothing else from the host is visible to the build, which is the point: the
# result does not depend on what happens to be installed on the build machine.
set -e
WS="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SDP="${QNX_SDP_ROOT:?set QNX_SDP_ROOT to your QNX SDP 8.0 installation}"
[ -f "$SDP/qnxsdp-env.sh" ] || { echo "no QNX SDP at $SDP" >&2; exit 1; }
[ -d "$HOME/.qnx" ] || { echo "no QNX license directory ~/.qnx -- activate the SDP license first" >&2; exit 1; }
IMAGE="${OSS_BUILD_IMAGE:-openqnx/oss-build}"

if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
  docker build -t "$IMAGE" --build-arg UID="$(id -u)" --build-arg GID="$(id -g)" "$WS/docker"
fi

tty=(); [ -t 0 ] && tty=(-it)
exec docker run --rm "${tty[@]}" \
  --user "$(id -u):$(id -g)" \
  -e HOME="$HOME" -e QNX_SDP_ROOT=/opt/qnx800 \
  -v "$WS:/work" \
  -v "$SDP:/opt/qnx800:ro" \
  -v "$HOME/.qnx:$HOME/.qnx:ro" \
  -w /work \
  "$IMAGE" "${@:-/bin/bash}"
