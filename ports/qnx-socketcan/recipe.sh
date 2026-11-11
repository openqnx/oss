# qnx-socketcan: Linux SocketCAN for programs on QNX 8.0.
#
# QNX has no SocketCAN: io-sock has no PF_CAN protocol family and there is
# no <linux/can.h>; QNX's CAN is a resource manager per controller
# (/dev/canN/..., devctl()s) whose drivers come with board BSPs. Programs
# written for SocketCAN (can-utils, python-can, ROS 2 CAN drivers, ...) open
# socket(PF_CAN, SOCK_RAW, CAN_RAW) and read and write struct can_frame.
# This component (its own source: files/) gives them that API in user space:
#
#   libqsocketcan.so   defines socket(), bind(), setsockopt(), read(),
#                      write(), recvmsg(), ioctl(), if_nametoindex(), ...:
#                      CAN sockets are served here, every other descriptor
#                      goes on to libsocket and libc
#   qcand              the CAN bus daemon: virtual CAN interfaces (as Linux's
#                      vcan; default vcan0, CAN FD capable), frames from one
#                      socket to the others with CAN_RAW's filters, loopback,
#                      own-message and error-mask rules
#   qcanctl            list, add, remove interfaces (ip link ... type vcan)
#   <linux/can.h>, <linux/can/raw.h>, <linux/can/error.h>  the Linux UAPI
#                      headers (from can-utils' copy of them), with the
#                      <linux/types.h>, <linux/socket.h>, <linux/stddef.h>
#                      they include -- in include/qnx-socketcan/, off the
#                      default include path: `pkg-config qnx-socketcan`
#
# Link order: -lqsocketcan before -lsocket (pkg-config does that). Builds
# that name -lc ahead of it (this workspace's CMake toolchain) work too: a
# message on a CAN socket's connection to qcand is the CAN frame as it is,
# so libc's read()/write() carry frames, and <linux/socket.h> redirects
# ioctl(); what differs then is listed in files/src/qsocketcan.c. A real
# PF_CAN protocol family is out of reach: SDP 8.0 has no public
# <sys/sockmsg.h> to serve /dev/socket/<af> with.
#
# Scope: CAN_RAW with classic CAN and CAN FD frames on virtual interfaces.
# Not there: CAN_BCM, CAN_ISOTP, CAN_J1939 (Linux kernel protocols), CAN XL,
# and a bridge to a hardware controller's /dev/canN (docs/STATUS.md).
VERSION=1.0
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-socketcan/files/." "$SRCROOT/"; }
qs_cc() { "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -D_QNX_SOURCE -DQCAN_NO_IOCTL_REDIRECT -O2 -Wall -Wno-missing-braces -I"$S/include" -I"$S/src" "$@"; }
do_build() {
  qs_cc -fPIC -shared -Wl,-soname,libqsocketcan.so.1 "$S/src/qsocketcan.c" -o "$B/libqsocketcan.so.1" -lsocket || return 1
  qs_cc "$S/src/qcand.c" -o "$B/qcand" -lsocket || return 1
  qs_cc "$S/src/qcanctl.c" -o "$B/qcanctl" -lsocket || return 1
}
do_install() {
  local inc="$STAGE$PREFIX/include/qnx-socketcan/linux"
  install -Dm755 "$B/libqsocketcan.so.1" "$STAGE$PREFIX/lib/libqsocketcan.so.1" || return 1
  ln -sf libqsocketcan.so.1 "$STAGE$PREFIX/lib/libqsocketcan.so"
  install -Dm755 "$B/qcand" "$STAGE$PREFIX/bin/qcand" || return 1
  install -Dm755 "$B/qcanctl" "$STAGE$PREFIX/bin/qcanctl" || return 1
  install -d "$inc/can" "$STAGE$PREFIX/lib/pkgconfig" "$STAGE$PREFIX/share/licenses/qnx-socketcan"
  install -m644 "$S"/include/linux/*.h "$inc/" || return 1
  install -m644 "$S"/include/linux/can/*.h "$inc/can/" || return 1
  sed -e "s|@PREFIX@|$PREFIX|" -e "s|@VERSION@|$VERSION|" "$S/qnx-socketcan.pc.in" > "$STAGE$PREFIX/lib/pkgconfig/qnx-socketcan.pc"
  install -m644 "$S/BSD-3-Clause" "$S"/*.txt "$STAGE$PREFIX/share/licenses/qnx-socketcan/" || return 1
}
