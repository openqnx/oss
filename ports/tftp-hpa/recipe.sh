VERSION=5.2; GIT=https://git.kernel.org/pub/scm/network/tftp/tftp-hpa.git; REF=tftp-hpa-5.2
# Also linked at run time (found by scripts/gen-dependency-index): readline
DEPS="readline"
BUILDSYS=custom
# tftp-hpa's Makefiles only work in the source tree, so build in a copy of it.
# -fcommon: tftp.c and main.c both define "toplevel" (old-style common symbol),
# an error by default since GCC 10.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && autoreconf -fi && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE -fcommon" LIBS="-lsocket" ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" \
     --without-tcpwrappers --without-ipv6; }
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" INSTALLROOT="$STAGE" install; }
