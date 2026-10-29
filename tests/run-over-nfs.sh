#!/bin/bash
# Run the suite on a target that mounts this workspace over NFS, instead of
# copying the bundle to it.
#
#   ./tests/run-over-nfs.sh <arch> <host> [user] [bundle-dir]
#
# For images too small to hold the bundle.  A default mkqnximage VM has a
# 146 MB disk, and /dev/shmem there accepts neither directories nor execution,
# so the only way to run the larger tests (vsomeip's closure alone is 181 MB)
# is to leave them on the build machine.  This host already exports its home
# directory to the VM subnet; the target mounts it READ-ONLY with fs-nfs3
# (-r), which needs root on the target.  The tests write only to /tmp.
#
# If the image also lacks QNX's own split-libc libraries (libeventfd, ...),
# they are copied from the SDP into <bundle>/oslib and put on the path -- the
# same opt-in as deploy-and-run-chunked.sh --with-os-runtime.
set -o pipefail
WS="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ARCH="$1"; HOST="$2"; USER_="${3:-root}"
BUNDLE="${4:-$WS/dist/qnx-port-tests-$ARCH}"
[ -n "$ARCH" ] && [ -n "$HOST" ] || { echo "usage: $0 <arch> <host> [user] [bundle-dir]" >&2; exit 2; }
[ -d "$BUNDLE/bin" ] || { echo "!! no bundle at $BUNDLE (run tests/make-bundle.sh)" >&2; exit 1; }
source "$WS/env.sh" "$ARCH" >/dev/null || exit 1

EXPORT="${NFS_EXPORT:-$HOME}"          # what the host exports
SERVER="${NFS_SERVER:-192.168.122.1}"  # this host, as the target sees it
MNT=/mnt/qnx-port-host   # fs-nfs3 creates it; / is a read-only IFS, so no mkdir
case "$BUNDLE" in "$EXPORT"/*) ;; *) echo "!! $BUNDLE is not under the export $EXPORT" >&2; exit 1;; esac
REMOTE_BUNDLE="$MNT/${BUNDLE#$EXPORT/}"

# Copy QNX's split-libc runtime next to the bundle only if the target lacks it.
source "$WS/tests/ssh-opts.sh"
mapfile -t SSHOPT < <(qnxport_ssh_opts "$WS")
if ! ssh "${SSHOPT[@]}" "$USER_@$HOST" 'for d in /lib /usr/lib /system/lib /proc/boot; do [ -e $d/libeventfd.so.1 ] && exit 0; done; exit 1'; then
  mkdir -p "$BUNDLE/oslib"
  for l in libsocket libeventfd libepoll libsignalfd libtimerfd libfsnotify libregex libm libasound; do
    for d in "$QNX_TARGET_SDP/$QNX_ARCH/lib" "$QNX_TARGET_SDP/$QNX_ARCH/usr/lib"; do
      for f in "$d/$l".so.[0-9]*; do [ -e "$f" ] && cp --update=none "$f" "$BUNDLE/oslib/"; done
    done
  done
  echo "-- target lacks QNX's split-libc runtime; carrying $(ls "$BUNDLE/oslib" | wc -l) files from the SDP"
fi

echo "== $ARCH -> $USER_@$HOST, bundle over NFS: $SERVER:$EXPORT (read-only)"
ssh "${SSHOPT[@]}" "$USER_@$HOST" "
  [ -d $REMOTE_BUNDLE ] || { fs-nfs3 -r $SERVER:$EXPORT $MNT && sleep 2; }
  [ -d $REMOTE_BUNDLE/bin ] || { echo '!! mount failed'; exit 1; }
  cd $REMOTE_BUNDLE &&
  { [ -d oslib ] && LD_LIBRARY_PATH=\$PWD/oslib && export LD_LIBRARY_PATH; }
  ./bin/run-on-target.sh"
