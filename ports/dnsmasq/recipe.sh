VERSION=2.93test4; GIT=https://thekelleys.org.uk/git/dnsmasq.git; REF=v2.93test4
BUILDSYS=custom
do_build() { cp -a "$S/." "$B/" && make -C "$B" CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -D_QNX_SOURCE" LDFLAGS="-lsocket -lm" PREFIX="$PREFIX"; }
do_install() { make -C "$B" PREFIX="$PREFIX" DESTDIR="$STAGE" install; }
