# perl-protocol-websocket: Protocol::WebSocket, the WebSocket protocol (handshake and frames, RFC 6455) for Perl.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=0.26
TARBALL=https://cpan.metacpan.org/authors/id/V/VT/VTI/Protocol-WebSocket-0.26.tar.gz
SHA256=5837d03711a7a3254f0afecb7e409e8b09376060c38a5b8295e8ee9af7575522
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
