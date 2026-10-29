# vincenthz/libjson: a small streaming JSON parser/printer in C. No releases;
# pinned to the last commit.
VERSION=0.8-git-20260512; GIT=https://github.com/vincenthz/libjson.git; COMMIT=da08310c7a7b7919205425dfa9469d89accb7aee
BUILDSYS=custom
do_build() { cp -a "$S/." "$B/" && make -C "$B" CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -fPIC -D_QNX_SOURCE" PREFIX="$PREFIX"; }
do_install() { make -C "$B" PREFIX="$PREFIX" DESTDIR="$STAGE" install-lib; }
