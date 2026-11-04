# perl-uri: URI, uniform resource identifiers (parse, build, normalise, resolve) for Perl.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=5.40
TARBALL=https://cpan.metacpan.org/authors/id/O/OA/OALDERS/URI-5.40.tar.gz
SHA256=ef5d2ef099a1a24a6e83ffe543de73337805d3d39c00b84fa205d725ad1a1b63
DEPS="perl"
RUNTIME_DEPS="perl perl-mime-base32"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
