# can-utils, the Linux SocketCAN user-space tools -- only the ones that do
# not need a CAN socket. QNX 8.0 has no SocketCAN: io-sock has no PF_CAN /
# AF_CAN protocol family (<sys/socket.h> stops at AF_DIVERT; no <linux/can.h>,
# no can interfaces), and QNX's CAN stack is a resource manager instead
# (dev-can-* drivers, /dev/canN/{rx,tx}M, devctl()s from <sys/can_dcmd.h>,
# canctl) whose drivers come with board BSPs -- none in the SDP. candump,
# cansend, cangen, canplayer, the ISO-TP and J1939 tools all open
# socket(PF_CAN, ...), so they cannot run on QNX and are not built.
# What is built works on CAN data, not on a CAN bus:
#   asc2log, log2asc   convert between candump log files and Vector ASC
#   log2long           print a candump log in the long (candump -ta) form
#   can-calc-bit-timing  bit-timing registers for CAN controllers from a
#                      bitrate and a clock (no device access)
VERSION=2025.01; GIT=https://github.com/linux-can/can-utils.git; REF=v2025.01
BUILDSYS=custom
CU_TOOLS="asc2log log2asc log2long can-calc-bit-timing"
# The tools include the Linux UAPI headers can-utils ships (include/linux/
# can.h, ...), which need <linux/types.h>, <linux/socket.h> and
# <linux/stddef.h>; qnx/linux/ (added by the patch) maps those onto QNX's
# own types. Nothing here opens a socket.
cu_cc() { "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall \
            -I"$S/qnx" -I"$S/include" "$@"; }
do_build() {
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
