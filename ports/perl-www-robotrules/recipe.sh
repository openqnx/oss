# perl-www-robotrules: WWW::RobotRules, parses robots.txt files for Perl web robots.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=6.03
TARBALL=https://cpan.metacpan.org/authors/id/O/OA/OALDERS/WWW-RobotRules-6.03.tar.gz
SHA256=8522b532935a11bfa688c2e113bac66729df4851be50c2c26d4b06f45fade472
DEPS="perl"
RUNTIME_DEPS="perl perl-uri"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
