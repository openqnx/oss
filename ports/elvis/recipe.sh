# elvis, the vi clone, from its maintained repository (the 2.2_1 line: 65
# commits of fixes after the last tag, v2.2_1-pre3).
VERSION=2.2.1-git-20240226; GIT=https://github.com/mbert/elvis.git; COMMIT=3684829e8596b9761c36629f20ff9d3262c10481
DEPS="ncurses"
BUILDSYS=custom
# elvis's configure is hand-written: given the system name "qnx6" (QNX
# Neutrino, which 8.0 still is) it writes config.h and the Makefile without
# running anything. Its header probes read the build machine's /usr/include,
# which agrees with QNX here (termios, select, sockets, setpgid). The termcap
# interface uses ncurses' termcap API (tgetent and friends in libncursesw,
# <termcap.h> in include/ncursesw), not the -ltermcap of QNX 6.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && sh ./configure --with-x=no --without-gcc --prefix="$PREFIX" qnx6 && \
  sed -i "s!^CC=.*!CC=qcc -V$QCC_TARGET -D_QNX_SOURCE -I$STAGE$PREFIX/include/ncursesw -I$STAGE$PREFIX/include!; \
          s!^LIBS=.*!LIBS=-L$STAGE$PREFIX/lib -lncursesw -lsocket!" Makefile; }
do_build() { make -C "$B" -j6; }
# The install also writes $DESTDIR/etc/elvis/README, a note for an
# administrator's /etc/elvis overrides; a package must not own files in the
# target's /etc, so it goes (elvis still reads /etc/elvis when it exists).
# It runs the just-built elvtags to index data/stubs: on the build machine
# that falls back to the host's ctags, or leaves no tags file, as upstream
# allows ("-" in the Makefile).
do_install() { make -C "$B" DESTDIR="$STAGE" install && rm -rf "$STAGE/etc/elvis" && rmdir "$STAGE/etc" 2>/dev/null; :; }
