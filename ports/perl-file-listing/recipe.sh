# perl-file-listing: File::Listing, parses directory listings (ls -l, FTP LIST output) for Perl.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=6.16
TARBALL=https://cpan.metacpan.org/authors/id/P/PL/PLICEASE/File-Listing-6.16.tar.gz
SHA256=189b3a13fc0a1ba412b9d9ec5901e9e5e444cc746b9f0156d4399370d33655c6
DEPS="perl"
RUNTIME_DEPS="perl perl-http-date"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
