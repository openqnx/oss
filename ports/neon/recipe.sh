# neon: HTTP and WebDAV client library (sessions, request/response, auth,
# PROPFIND/PROPPATCH, locking), with SSL/TLS through OpenSSL, XML through
# expat and gzip content decoding through zlib.
# The release tarball: it ships the generated configure (the git tree needs
# autogen.sh and the docs toolchain).
VERSION=0.37.1
TARBALL=https://notroj.github.io/neon/neon-0.37.1.tar.gz
SHA256=a99b7262525a454d1065cf76dd17240fd808dfc4ef15636990ff83a5d0d9e740
BUILDSYS=autotools
DEPS="openssl expat zlib"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-ssl=openssl --with-expat
                  --without-libproxy --without-gssapi --disable-nls --disable-debug )
# The tarball carries no generated man pages (doc/man is the XML source,
# built with xmlto), and `make install` stops on the missing doc/man/*.3:
# install the library, headers and neon-config/neon.pc only.
do_install() { make -C "$B" DESTDIR="$STAGE" install-lib install-headers install-config; }
