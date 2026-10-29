# antirez/dump1090 -- the original Mode S decoder for RTL-SDR dongles; this is
# the tree the qnx-ports port is based on. No releases; pinned commit.
VERSION=0-git-20240121; GIT=https://github.com/antirez/dump1090.git; COMMIT=0c3bb23eb447d4ae47c7013346fa6fa97482bb1d
DEPS="rtl-sdr"
BUILDSYS=custom
do_build() { cp -a "$S/." "$B/" && make -C "$B" CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -D_QNX_SOURCE -DQNX=1 -I$STAGE$PREFIX/include" \
  LDLIBS="-L$STAGE$PREFIX/lib -lrtlsdr -lm -lsocket"; }
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/dump1090" "$STAGE$PREFIX/bin/"; }
