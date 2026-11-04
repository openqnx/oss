# pinentry: the PIN/passphrase entry dialogs that gpg-agent (and other
# Assuan clients) drive. The terminal flavours: pinentry-curses (also the
# default `pinentry`) and pinentry-tty; the GTK/Qt/GNOME ones need a
# desktop session and stay off, as does libsecret caching.
VERSION=1.3.3
TARBALL=https://gnupg.org/ftp/gcrypt/pinentry/pinentry-1.3.3.tar.bz2
SHA256=c2970f16d6afb66ecddfca767d743936c86239bff936eed7fd7597a678414b63
DEPS="libassuan libgpg-error ncurses libiconv"
BUILDSYS=autotools
# --disable-rpath: lib-link.m4 links libiconv by its staging path and would
# record that build-machine directory as the programs' RPATH.
CONF_ARGS_EXTRA=( --disable-rpath --enable-pinentry-curses --enable-pinentry-tty --enable-fallback-curses
                  --disable-pinentry-gtk2 --disable-pinentry-gnome3 --disable-pinentry-qt
                  --disable-pinentry-qt5 --disable-pinentry-emacs --disable-inside-emacs
                  --disable-pinentry-fltk --disable-pinentry-efl --disable-libsecret
                  --with-libgpg-error-prefix="$STAGE$PREFIX" --with-libassuan-prefix="$STAGE$PREFIX" )
# gpg-error.m4/libassuan.m4 must ask the staged gpgrt-config, not the build
# machine's (as ports/libassuan does).
CONF_ENV=( GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" )
