# perl-mozilla-ca: Mozilla::CA, Mozilla's CA certificate bundle for Perl
# (LWP and IO::Socket::SSL verify HTTPS servers with it). Pure Perl and a
# PEM file: installed into Perl's site library.
VERSION=20261105
TARBALL=https://cpan.metacpan.org/authors/id/L/LW/LWP/Mozilla-CA-20261105.tar.gz
SHA256=b29dfd05d0f0038091fab9d7a184e8e476bc24fb5de5963cd23d21d262e0be10
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_configure() { :; }
do_build() { :; }
do_install() {
  local pv; pv=$(sed -n 's/^VERSION=\([^;]*\).*/\1/p' "$WS/ports/perl/recipe.sh")
  local d="$STAGE$PREFIX/lib/perl5/site_perl/$pv"; mkdir -p "$d"
  cp -a "$S"/*/lib/Mozilla "$d/"
}
