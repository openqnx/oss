# libcanard: the OpenCyphal/CAN (and DroneCAN, the former UAVCAN v0)
# protocol stack in C for embedded and real-time systems: transfers are
# split into and reassembled from classic CAN and CAN FD frames, with
# priorities, multi-frame CRC, transfer-ID deduplication, redundant
# interfaces and node-ID allocation. It has no I/O of its own: the
# application hands it received frames (canard_ingest_frame()) and sends the
# frames it asks for (the tx vtable), here through qnx-socketcan's CAN_RAW.
#
# Upstream distributes two files to copy into a project (canard.c, canard.h;
# its CMake file is for library development: clang-tidy, cppcheck, tests).
# Here they are a library: libcanard.so.5 and libcanard.a, canard.h and
# libcanard.pc. The AVL tree header (lib/cavl2) is internal to canard.c.
VERSION=5.0.0; GIT=https://github.com/OpenCyphal/libcanard.git; REF=v5.0.0
DEPS=""
BUILDSYS=custom
lc_cc() { "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -D_QNX_SOURCE -std=gnu11 -O2 -fPIC -Wall -Wextra -I"$S/libcanard" -I"$S/lib/cavl2" "$@"; }
do_build() {
  lc_cc -c "$S/libcanard/canard.c" -o "$B/canard.o" || return 1
  rm -f "$B/libcanard.a"; "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs "$B/libcanard.a" "$B/canard.o" || return 1
  lc_cc -shared -Wl,-soname,libcanard.so.5 "$B/canard.o" -o "$B/libcanard.so.5"
}
do_install() {
  install -Dm755 "$B/libcanard.so.5" "$STAGE$PREFIX/lib/libcanard.so.5" || return 1
  ln -sf libcanard.so.5 "$STAGE$PREFIX/lib/libcanard.so"
  install -Dm644 "$B/libcanard.a" "$STAGE$PREFIX/lib/libcanard.a" || return 1
  install -Dm644 "$S/libcanard/canard.h" "$STAGE$PREFIX/include/canard.h" || return 1
  install -Dm644 "$S/LICENSE" "$STAGE$PREFIX/share/licenses/libcanard/LICENSE" || return 1
  install -d "$STAGE$PREFIX/lib/pkgconfig"
  cat > "$STAGE$PREFIX/lib/pkgconfig/libcanard.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: libcanard
Description: OpenCyphal/CAN and DroneCAN protocol stack in C
Version: $VERSION
Libs: -L\${libdir} -lcanard
Cflags: -I\${includedir}
PC
}
