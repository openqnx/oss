# perl-fcgi: FCGI, the FastCGI application interface for Perl (XS, bundles the fcgi client library).
# Makefile.PL runs the bundled autoconf configure (for fcgi_config.h) with
# the build machine perl's compiler unless fcgi_config.h exists: that
# described Linux (no sockaddr_un.sun_len, which QNX has). configure is run
# here first, as a cross configure for QNX, and Makefile.PL keeps its result.
VERSION=0.82
TARBALL=https://cpan.metacpan.org/authors/id/E/ET/ETHER/FCGI-0.82.tar.gz
SHA256=4c7d60e26da2c07f058a4e345021e92505273b33c9542215977e084611f09ecf
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() {
  mkdir -p "$B" && cp -a "$S/." "$B/" && cd "$B" && \
  CC="qcc -V$QCC_TARGET" ./configure --host="$QNX_TRIPLE" --build="$(uname -m)-pc-linux-gnu" && \
  qnx_perl_xs_configure
}
do_build() { qnx_perl_xs_make; }
do_install() { qnx_perl_xs_install; }
