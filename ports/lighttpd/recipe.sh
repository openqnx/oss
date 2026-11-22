VERSION=1.4.85; GIT=https://git.lighttpd.net/lighttpd/lighttpd1.4.git; REF=lighttpd-1.4.85
DEPS="openssl pcre2 zlib"
# lighttpd asks find_package(PCRE2) for no component, which pcre2's own config
# file refuses (a fatal error, not a quiet miss): take its pkg-config route.
CMAKE_ARGS_EXTRA=( -DCMAKE_DISABLE_FIND_PACKAGE_PCRE2=ON -DWITH_OPENSSL=ON -DWITH_PCRE2=ON -DWITH_ZLIB=ON -DWITH_BZIP=OFF -DBUILD_STATIC=OFF )
