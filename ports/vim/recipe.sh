VERSION=9.1.0900; GIT=https://github.com/vim/vim.git; REF=v9.1.0900
# Also linked at run time (found by scripts/gen-dependency-index): libiconv libsodium
DEPS="ncurses libiconv libsodium"
BUILDSYS=autotools
# vim's configure probes several things by running test programs; these are the
# answers for QNX, which configure cannot find out itself when cross compiling.
# vim calls socket functions (for its server/clipboard code) that live in
# QNX's libsocket; configure does not know to link it.
CONF_ENV=( vim_cv_toupper_broken=no vim_cv_terminfo=yes vim_cv_tgetent=zero vim_cv_getcwd_broken=no
           vim_cv_stat_ignores_slash=no vim_cv_memmove_handles_overlap=yes vim_cv_bcopy_handles_overlap=yes
           vim_cv_memcpy_handles_overlap=no ac_cv_small_wchar_t=no vim_cv_uname_output=QNX vim_cv_uname_r_output=8.0.0
           vim_cv_uname_m_output=unknown )
CONF_ARGS_EXTRA=( --with-features=huge --enable-gui=no --without-x --disable-nls --with-tlib=ncursesw
                  --disable-netbeans --disable-channel --enable-multibyte )
SUBDIR=
do_configure() { cd "$B" && cp -a "$SRCROOT/." "$B/" && cd "$B" && \
  env CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CPPFLAGS="-I$STAGE$PREFIX/include -I$STAGE$PREFIX/include/ncursesw" \
      LDFLAGS="-L$STAGE$PREFIX/lib" LIBS="-lsocket" "${CONF_ENV[@]}" ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" "${CONF_ARGS_EXTRA[@]}"; }
do_build() { make -C "$B" -j"$(nproc)"; }
# vim strips the binary it installs; with the host strip that fails on the
# other architecture ("Unable to recognise the format").
do_install() { make -C "$B" DESTDIR="$STAGE" STRIP="$QNX_HOST/usr/bin/nto${QNX_CPU}-strip" install; }
