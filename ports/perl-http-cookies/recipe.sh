# perl-http-cookies: HTTP::Cookies, the HTTP cookie jar of libwww-perl (LWP).
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=6.12
TARBALL=https://cpan.metacpan.org/authors/id/O/OA/OALDERS/HTTP-Cookies-6.12.tar.gz
SHA256=4e460c4bae76285bfc726f641349402fc9038fcd9ca0f33346bec036d73876b8
DEPS="perl"
RUNTIME_DEPS="perl perl-http-date perl-http-message"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
