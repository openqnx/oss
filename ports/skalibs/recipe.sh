# skalibs, skarnet's general-purpose C library (buffers, stralloc, iopause,
# selfpipe, TAI time, djb-style unix utilities) that s6, execline and the
# rest of the skarnet software are built on. 2.14.5.1 with qnx-ports' QNX
# change (patches/0001: configure finds the byte order in <sys/param.h>,
# where QNX keeps BYTE_ORDER -- there is no <endian.h>); patches/0002:
# SA_RESTART, which QNX does not implement, is 0 in sig_catch().
VERSION=2.16.0.0; GIT=https://github.com/skarnet/skalibs.git; REF=v2.16.0.0
BUILDSYS=custom
# skalibs' configure is hand-written and works in the source tree only; it
# is driven with the cross-GCC (in PATH from env.sh) and the target triple,
# so it finds the matching ar/ranlib/strip by prefix. The three sysdeps it
# cannot detect when cross-compiling: /dev/urandom works (QNX's random
# service); posix_spawn() reports exec failure (no early return); and there
# is no readlink()-able /proc/self/exe (QNX's /proc/self/exefile is a plain
# file), so procselfexe=none. 2.16 adds selectinfinite (does select() take a
# timeout over 10^8 s): =no caps such timeouts at 10^8 s, which is always
# correct; skalibs uses ppoll() on QNX anyway, select() is only the fallback.
# Everything else is probed by compiling and linking against the SDP.
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && \
  CC="$QNX_TRIPLE-gcc" CFLAGS="-O2 -D_QNX_SOURCE" LDFLAGS="-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state" \
  ./configure --target="$QNX_TRIPLE" --prefix="$PREFIX" --enable-shared --enable-static --enable-pkgconfig \
    --with-sysdep-devurandom=yes --with-sysdep-posixspawnearlyreturn=no --with-sysdep-procselfexe=none \
    --with-sysdep-selectinfinite=no
}
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
