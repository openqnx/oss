#!/bin/bash
# Ship a bundle to a live QNX target and run the suite there.
#
#   ./tests/deploy-and-run.sh <arch> <host>
#
# Unpacks under $HOME on the target, not /tmp: on the x86_64 QEMU image /tmp is
# a small RAM disk and a 170 MB bundle closes the connection mid-copy
# ("scp: Connection closed"), which looks like a network fault and is not one.
set -o pipefail
WS="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ARCH="$1"; HOST="$2"
[ -n "$ARCH" ] && [ -n "$HOST" ] || { echo "usage: $0 <arch> <host>" >&2; exit 2; }
TAR="$WS/dist/qnx-port-tests-$ARCH.tar.gz"
[ -f "$TAR" ] || { echo "!! no bundle: $TAR (run tests/make-bundle.sh)" >&2; exit 1; }

# Target account; a stock image has qnxuser, a mkqnximage VM with --ssh-ident
# only lets root in with the key.
TUSER="${TARGET_USER:-qnxuser}"
source "$WS/tests/ssh-opts.sh"
mapfile -t SSHOPT < <(qnxport_ssh_opts "$WS")

echo "== $ARCH -> $TUSER@$HOST"
scp "${SSHOPT[@]}" -q "$TAR" "$TUSER@$HOST:" || exit 1
ssh "${SSHOPT[@]}" "$TUSER@$HOST" "
  cd \$HOME &&
  rm -rf qnx-port-tests-$ARCH &&
  tar xzf qnx-port-tests-$ARCH.tar.gz &&
  cd qnx-port-tests-$ARCH &&
  ./bin/run-on-target.sh"
