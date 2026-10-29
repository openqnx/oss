VERSION=1.2.4; GIT=https://gitlab.com/NTPsec/ntpsec.git; REF=NTPsec_1_2_4
DEPS="openssl"
BUILDSYS=custom
# NTPsec builds with waf (Python). Cross compiling is a waf feature: the
# compiler and flags for the target are passed as --cross-* options, and waf
# itself runs on the build host.
# The QNX patch recognises a QNX build by "qnx" appearing in the compiler's
# name (it skips run-time probes and the struct timex check then). Call the
# compiler through a wrapper named qnx-qcc, so that holds wherever the SDP is.
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && \
  printf '#!/bin/sh\nexec qcc "$@"\n' > "$B/qnx-qcc" && chmod +x "$B/qnx-qcc" && \
  env -u CC -u CFLAGS -u LDFLAGS -u CPPFLAGS PKG_CONFIG_LIBDIR="$PKG_CONFIG_LIBDIR" \
  python3 ./waf configure --prefix="$PREFIX" \
    --cross-compiler="$B/qnx-qcc -V$QCC_TARGET" \
    --cross-cflags="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
    --cross-ldflags="-L$STAGE$PREFIX/lib -lsocket -lm" \
    --disable-mdns-registration --disable-doc --disable-manpage
}
do_build() { cd "$B" && python3 ./waf build -j"$(nproc)"; }
do_install() { cd "$B" && python3 ./waf install --destdir="$STAGE"; }
