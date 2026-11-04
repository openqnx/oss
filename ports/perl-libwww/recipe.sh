# perl-libwww: libwww-perl (LWP), the World-Wide Web library for Perl: LWP::UserAgent and the lwp-* tools.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=6.83
TARBALL=https://cpan.metacpan.org/authors/id/O/OA/OALDERS/libwww-perl-6.83.tar.gz
SHA256=e75f0fa9d3c6f0daf5a5a72fa9f8b1c9c0d23e3a84a8522ccb4f835232b95505
DEPS="perl"
RUNTIME_DEPS="perl perl-encode-locale perl-file-listing perl-html-parser perl-http-cookies perl-http-date perl-http-message perl-http-negotiate perl-lwp-mediatypes perl-net-http perl-try-tiny perl-uri perl-www-robotrules"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install bin/lwp-download bin/lwp-dump bin/lwp-mirror bin/lwp-request; }
