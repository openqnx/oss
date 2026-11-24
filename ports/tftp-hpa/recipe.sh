VERSION=7.2; GIT=https://git.kernel.org/pub/scm/network/tftp/tftp-hpa.git; REF=tftp-hpa-7.2
# Also linked at run time (found by scripts/gen-dependency-index): readline
DEPS="readline ncurses"
BUILDSYS=custom
# tftp-hpa's Makefiles only work in the source tree, so build in a copy of it.
# The git tree has no configure: autogen.sh makes it (and the autoconf
# helpers and config.h.in). IPv6 stays off as in the 5.2 port.
# configure puts "-MMD -MF ${DEPENDFILE}" (dependency files) in CFLAGS, which
# the Makefiles pass to the link step too; gcc ignores it there, but qcc hands
# the .dep name to ld ("cannot find tftp.dep"). Take it out of config/MCONFIG.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && ./autogen.sh && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" LIBS="-lsocket" ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" \
     --disable-ipv6 && \
  sed -i 's/ -MMD -MF \${DEPENDFILE}//' config/MCONFIG && ! grep -q DEPENDFILE config/MCONFIG; }
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" INSTALLROOT="$STAGE" install; }
