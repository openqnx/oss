VERSION=1.4.0
TARBALL=https://github.com/mobile-shell/mosh/releases/download/mosh-1.4.0/mosh-1.4.0.tar.gz
SHA256=872e4b134e5df29c8933dff12350785054d2fd2839b5ae6b5587b14db1465ddd
BUILDSYS=autotools
DEPS="protobuf ncurses openssl zlib"
# mosh generates its wire-format classes with protoc at build time: that has to
# be the build machine's protoc of the same version as the staged runtime
# (protobuf 29 gencode checks PROTOBUF_VERSION exactly) -- the protobuf
# recipe's host build.
# configure asks for C++11 for any protobuf >= 3.6 and appends -std=gnu++11 to
# CXX; protobuf 29 and abseil need C++17. CXXFLAGS comes after it on every
# compile line, so -std=gnu++17 there wins (the rest repeats qnxbuild's
# CXXFLAGS, which this assignment replaces).
CONF_ENV=( PROTOC="$BUILD_DIR/host/protobuf/protoc"
           CXXFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include -std=gnu++17" )
# OpenSSL for AES-OCB (the staged one, not the SDP's); the bash completion and
# the perl wrapper's ssh path are not specific to QNX and stay as upstream.
CONF_ARGS_EXTRA=( --with-crypto-library=openssl --enable-client --enable-server
                  --disable-examples )
# protobuf's pkg-config file lists every abseil library it was built with;
# mosh calls into a handful. --as-needed keeps the rest out of the programs'
# NEEDED lists (libc stays needed: qnxbuild names it first under --no-as-needed).
LDFLAGS_APPEND="-Wl,--as-needed"
