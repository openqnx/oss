VERSION=1.0.11
TARBALL=https://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-1.0.11.tar.gz
SHA256=f227a044b75688e449b58ddedf0582255cc562899ff69f173a8ef76439744b4e
BUILDSYS=autotools
# QNX 8 epoll (libepoll) has no EPOLLET or EPOLLRDHUP, both of which the epoll
# backend relies on; the poll() backend is used instead.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc --disable-examples --disable-tools
                  --disable-https --without-gnutls
                  --disable-epoll )
