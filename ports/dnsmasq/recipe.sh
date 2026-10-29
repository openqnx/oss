VERSION=2.93test4; GIT=https://thekelleys.org.uk/git/dnsmasq.git; REF=v2.93test4
BUILDSYS=custom
# The same make variables for "install": its target depends on "all", and
# without CC there it rebuilt dnsmasq with the build host's cc -- both
# architectures shipped an x86-64 Linux executable (scripts/check-artifacts).
_dnsmasq_make() { make -C "$B" CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -D_QNX_SOURCE" LDFLAGS="-lsocket -lm" PREFIX="$PREFIX" "$@"; }
do_build() { cp -a "$S/." "$B/" && _dnsmasq_make; }
do_install() { _dnsmasq_make DESTDIR="$STAGE" install; }
