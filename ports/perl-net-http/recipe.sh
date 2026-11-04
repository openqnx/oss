# perl-net-http: Net::HTTP, the low-level HTTP/1.1 client connection class of libwww-perl.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=6.24
TARBALL=https://cpan.metacpan.org/authors/id/O/OA/OALDERS/Net-HTTP-6.24.tar.gz
SHA256=290ed9a97b05c7935b048e6d2a356035871fca98ad72c01c5961726adf85c83c
DEPS="perl"
RUNTIME_DEPS="perl perl-uri"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
