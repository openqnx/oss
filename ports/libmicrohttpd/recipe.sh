VERSION=1.0.1
TARBALL=https://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-1.0.1.tar.gz
SHA256=a89e09fc9b4de34dde19f4fcb4faaa1ce10299b9908db1132bbfa1de47882b94
BUILDSYS=autotools
# QNX 8 epoll (libepoll) has no EPOLLET or EPOLLRDHUP, both of which the epoll
# backend relies on; the poll() backend is used instead.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc --disable-examples --disable-tools
                  --disable-https --without-gnutls
                  --disable-epoll )
