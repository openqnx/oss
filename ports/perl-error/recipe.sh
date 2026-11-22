# Error.pm (try/catch exception objects for Perl; used by git's Perl
# scripts). Pure Perl, so the module files are installed as released into the
# target perl's library. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=0.17031
TARBALL=https://cpan.metacpan.org/authors/id/S/SH/SHLOMIF/Error-0.17031.tar.gz
SHA256=0a85374cf9cbde5a5075f16e62246e5d36abde63ec08c5c5694f128f4546852f
DEPS="perl"
BUILDSYS=custom
do_build() { :; }
# Into Perl's site library, site_perl/<perl version>: the versionless
# site_perl directory is not in @INC (the perl port has no inc_version_list),
# and `perl -MError` did not find the module there.
do_install() {
  local pv; pv=$(sed -n 's/^VERSION=\([^;]*\).*/\1/p' "$WS/ports/perl/recipe.sh")
  local d="$STAGE$PREFIX/lib/perl5/site_perl/$pv"
  install -d "$d/Error" && install -m644 "$S/lib/Error.pm" "$d/" && cp -a "$S/lib/Error/." "$d/Error/"
}
