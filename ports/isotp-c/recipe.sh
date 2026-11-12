# isotp-c: ISO 15765-2 (ISO-TP) transport for CAN in portable C: single
# frames, segmented messages with flow control, block size and STmin, CAN FD
# frame lengths. The stack is driven by the application (isotp_poll(),
# isotp_on_can_message()) and sends through three callbacks the application
# defines (isotp_user_send_can(), isotp_user_get_us(), isotp_user_debug()),
# so on QNX it runs over qnx-socketcan's CAN_RAW sockets, where Linux's
# CAN_ISOTP kernel sockets are not available.
#
# Upstream's CMake file has no install step and builds a shared library with
# the callbacks left undefined; here it is a static, position independent
# libisotp.a, the headers in include/isotp_c/ and isotp-c.pc. The compile
# time configuration is part of the interface (it sizes IsoTpLink and sets
# the callback signature), so isotp-c.pc carries the same definitions:
# frames up to 64 bytes (CAN FD) received, 8-byte frames sent by default
# (isotp_set_tx_dl() changes it per link), padding with 0xAA, and
# isotp_user_send_can(id, data, size, flags, arg) with the frame flags and
# the per-link argument (isotp_init_link(); link->user_send_can_arg).
VERSION=1.9.3; GIT=https://github.com/SimonCahill/isotp-c.git; REF=v1.9.3
DEPS=""
BUILDSYS=custom
ISOTP_DEFS="-DISO_TP_MAX_CAN_FRAME_SIZE=64 -DISO_TP_DEFAULT_TX_DL=8 -DISO_TP_FRAME_PADDING -DISO_TP_FRAME_PADDING_VALUE=0xAA -DISO_TP_USER_SEND_CAN_ARG -DISO_TP_USER_SEND_CAN_FLAGS"
do_build() {
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -fPIC -Wall -Wextra $ISOTP_DEFS -I"$S" -c "$S/isotp.c" -o "$B/isotp.o" || return 1
  rm -f "$B/libisotp.a"; "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs "$B/libisotp.a" "$B/isotp.o"
}
do_install() {
  install -Dm644 "$B/libisotp.a" "$STAGE$PREFIX/lib/libisotp.a" || return 1
  install -d "$STAGE$PREFIX/include/isotp_c" "$STAGE$PREFIX/lib/pkgconfig" "$STAGE$PREFIX/share/licenses/isotp-c"
  install -m644 "$S"/isotp.h "$S"/isotp_config.h "$S"/isotp_defines.h "$S"/isotp_user.h "$STAGE$PREFIX/include/isotp_c/" || return 1
  install -m644 "$S/LICENSE" "$STAGE$PREFIX/share/licenses/isotp-c/" || return 1
  cat > "$STAGE$PREFIX/lib/pkgconfig/isotp-c.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: isotp-c
Description: ISO 15765-2 (ISO-TP) transport for CAN in C
Version: $VERSION
Libs: -L\${libdir} -lisotp
Cflags: -I\${includedir}/isotp_c $ISOTP_DEFS
PC
}
