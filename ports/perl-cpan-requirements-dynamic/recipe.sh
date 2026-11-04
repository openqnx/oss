# perl-cpan-requirements-dynamic: CPAN::Requirements::Dynamic, dynamic prerequisites for Perl distributions (used by Module::Build::Tiny).
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=0.003
TARBALL=https://cpan.metacpan.org/authors/id/L/LE/LEONT/CPAN-Requirements-Dynamic-0.003.tar.gz
SHA256=0731694542ec57aead9f20259ba678f74e5602e655b1833eef3e9336f47298e8
DEPS="perl"
RUNTIME_DEPS="perl perl-extutils-config perl-extutils-hascompiler"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
