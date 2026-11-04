# perl-encode-locale: Encode::Locale, the encodings of the current locale and console for Perl.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=1.05
TARBALL=https://cpan.metacpan.org/authors/id/G/GA/GAAS/Encode-Locale-1.05.tar.gz
SHA256=176fa02771f542a4efb1dbc2a4c928e8f4391bf4078473bd6040d8f11adb0ec1
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
