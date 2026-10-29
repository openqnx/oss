VERSION=1.4.77; GIT=https://git.lighttpd.net/lighttpd/lighttpd1.4.git; REF=lighttpd-1.4.77
DEPS="openssl pcre2 zlib"
CMAKE_ARGS_EXTRA=( -DWITH_OPENSSL=ON -DWITH_PCRE2=ON -DWITH_ZLIB=ON -DWITH_BZIP=OFF -DBUILD_STATIC=OFF )
