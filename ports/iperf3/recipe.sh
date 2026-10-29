VERSION=3.17.1; GIT=https://github.com/esnet/iperf.git; REF=3.17.1
SRCROOT="$SRC_DIR/iperf3"
DEPS="openssl"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-openssl="$STAGE$PREFIX" )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
