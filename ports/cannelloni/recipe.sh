# cannelloni: a SocketCAN over Ethernet tunnel (UDP, TCP), CAN and CAN FD.
#
# Runs on qnx-socketcan (PF_CAN raw sockets, <linux/can.h>; buses from its
# daemon qcand): `cannelloni -I vcan0 -R <peer> -r 20000 -l 20000` on both
# ends joins two CAN buses. timerfd and signalfd come from QNX's libtimerfd
# and libsignalfd (no timerfd_gettime(): the patch keeps the timer state).
# Left out: SCTP transport (io-sock has no SCTP).
VERSION=2.1.2; GIT=https://github.com/mguentner/cannelloni.git; REF=v2.1.2
DEPS="qnx-socketcan"
RUNTIME_DEPS="qnx-socketcan"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_TESTING=OFF -DSCTP_SUPPORT=OFF
  "-DCMAKE_CXX_FLAGS=-I$STAGE$PREFIX/include/qnx-socketcan"
  "-DCMAKE_CXX_STANDARD_LIBRARIES=-L$STAGE$PREFIX/lib -lqsocketcan -lsocket -ltimerfd -lsignalfd" )
