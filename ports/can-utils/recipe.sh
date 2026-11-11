# can-utils, the Linux SocketCAN user-space tools.
#
# QNX 8.0 has no SocketCAN of its own (io-sock has no PF_CAN; QNX's CAN is a
# resource manager per controller). The tools here run on qnx-socketcan:
# its libqsocketcan gives them socket(PF_CAN, SOCK_RAW, CAN_RAW) and
# <linux/can.h>, and its daemon qcand the CAN buses (virtual ones, as
# Linux's vcan: start `qcand`, then `candump vcan0`, `cansend vcan0 123#11`).
#
# Built -- the CAN_RAW tools and the ones that work on CAN data:
#   candump, cansend, cangen, canplayer, cansniffer, canbusload,
#   cansequence, canfdtest, canlogserver
#   asc2log, log2asc, log2long    candump log files <-> Vector ASC, long form
#   can-calc-bit-timing           bit-timing registers for CAN controllers
# Not built: what needs a Linux kernel protocol or driver that qnx-socketcan
# does not provide -- bcmserver, cangw (CAN_BCM, CAN gateway via netlink),
# isotp* (CAN_ISOTP), j1939*/testj1939/isobusfs (CAN_J1939), slcan*
# (the slcan tty line discipline), mcp251xfd-dump.
VERSION=2025.01; GIT=https://github.com/linux-can/can-utils.git; REF=v2025.01
DEPS="qnx-socketcan"
BUILDSYS=custom
CU_SOCK_TOOLS="candump cansend cangen canplayer cansniffer canbusload cansequence canfdtest canlogserver"
CU_TOOLS="$CU_SOCK_TOOLS asc2log log2asc log2long can-calc-bit-timing"
# Include path: qnx/ (the patch: <sys/epoll.h> on poll()), qnx-socketcan's
# <linux/types.h>, <linux/socket.h>, <linux/sockios.h>, then the Linux UAPI
# CAN headers can-utils ships (include/linux/can.h, ...). libqsocketcan
# goes before libsocket: it defines socket(), bind(), read(), ... for CAN
# sockets and passes every other descriptor on.
cu_cc() { "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -D_QNX_SOURCE -D_GNU_SOURCE -O2 -Wall \
            -include "$S/qnx/qnx-compat.h" -I"$S/qnx" -I"$STAGE$PREFIX/include/qnx-socketcan" -I"$S/include" "$@"; }
do_build() {
  local t
  for t in $CU_SOCK_TOOLS; do
    cu_cc "$S/$t.c" "$S/lib.c" "$S/canframelen.c" -o "$B/$t" \
          -L"$STAGE$PREFIX/lib" -Wl,-rpath-link,"$STAGE$PREFIX/lib" -lqsocketcan -lsocket -lm || return 1
  done
  cu_cc "$S/asc2log.c"  "$S/lib.c" "$S/canframelen.c" -o "$B/asc2log"  || return 1
  cu_cc "$S/log2asc.c"  "$S/lib.c" "$S/canframelen.c" -o "$B/log2asc"  || return 1
  cu_cc "$S/log2long.c" "$S/lib.c" "$S/canframelen.c" -o "$B/log2long" || return 1
  cu_cc -I"$S/calc-bit-timing" "$S/calc-bit-timing/can-calc-bit-timing.c" -o "$B/can-calc-bit-timing"
}
do_install() {
  local t
  install -d "$STAGE$PREFIX/bin"
  for t in $CU_TOOLS; do install -m 755 "$B/$t" "$STAGE$PREFIX/bin/$t" || return 1; done
}
