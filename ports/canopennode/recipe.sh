# CANopenNode: the CANopen protocol stack (CiA 301, 303-3, 304, 305, 309-3),
# as its Linux application CANopenLinux: canopend, a CANopen device with the
# example object dictionary (NMT, heartbeat, SDO server and client, PDO,
# SYNC, TIME, emergency, LSS master and slave) and the CiA 309-3 ASCII
# command interface (stdio, local socket or TCP), and cocomm, the client
# of that interface.
#
# Runs on qnx-socketcan (PF_CAN raw sockets; buses from its daemon qcand):
# `canopend vcan0 -i 4 -c local-/tmp/CO_command_socket`, then
# `cocomm "1 r 0x1017 0 u16"`. epoll, eventfd and timerfd are QNX's
# libepoll, libeventfd and libtimerfd.
# The source is CANopenLinux v4.0 with the CANopenNode commit it pins.
VERSION=4.0; GIT=https://github.com/CANopenNode/CANopenLinux.git; REF=v4.0
DEPS="qnx-socketcan"
RUNTIME_DEPS="qnx-socketcan"
BUILDSYS=custom
CO_SRCS="CO_driver.c CO_error.c CO_epoll_interface.c CO_storageLinux.c CO_main_basic.c
  CANopenNode/301/CO_ODinterface.c CANopenNode/301/CO_NMT_Heartbeat.c CANopenNode/301/CO_HBconsumer.c
  CANopenNode/301/CO_Emergency.c CANopenNode/301/CO_SDOserver.c CANopenNode/301/CO_SDOclient.c
  CANopenNode/301/CO_TIME.c CANopenNode/301/CO_SYNC.c CANopenNode/301/CO_PDO.c
  CANopenNode/301/crc16-ccitt.c CANopenNode/301/CO_fifo.c CANopenNode/303/CO_LEDs.c
  CANopenNode/304/CO_GFC.c CANopenNode/304/CO_SRDO.c CANopenNode/305/CO_LSSslave.c
  CANopenNode/305/CO_LSSmaster.c CANopenNode/309/CO_gateway_ascii.c CANopenNode/storage/CO_storage.c
  CANopenNode/extra/CO_trace.c CANopenNode/CANopen.c CANopenNode/example/OD.c"
co_cc() { "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -g -Wall "$@"; }
do_build() {
  mkdir -p "$B/obj"
  local f o objs=""
  for f in $CO_SRCS; do
    o="$B/obj/$(echo "$f" | tr / _).o"; objs="$objs $o"
    co_cc -DCO_SINGLE_THREAD -I"$S" -I"$S/CANopenNode" -I"$S/CANopenNode/example" \
          -I"$STAGE$PREFIX/include/qnx-socketcan" -c "$S/$f" -o "$o" || return 1
  done
  # libqsocketcan before libsocket: it defines socket(), bind(), ... for CAN sockets.
  co_cc $objs -o "$B/canopend" -L"$STAGE$PREFIX/lib" -lqsocketcan -lsocket -lepoll -leventfd -ltimerfd || return 1
  co_cc -I"$STAGE$PREFIX/include/qnx-socketcan" "$S/cocomm/cocomm.c" -o "$B/cocomm" -L"$STAGE$PREFIX/lib" -lqsocketcan -lsocket
}
do_install() {
  mkdir -p "$STAGE$PREFIX/bin" "$STAGE$PREFIX/share/canopennode" "$STAGE$PREFIX/share/licenses/canopennode"
  cp "$B/canopend" "$B/cocomm" "$STAGE$PREFIX/bin/"
  cp "$S/CANopenNode/example/DS301_profile.eds" "$S/CANopenNode/example/DS301_profile.md" "$STAGE$PREFIX/share/canopennode/"
  cp "$S/LICENSE" "$STAGE$PREFIX/share/licenses/canopennode/LICENSE"
}
