VERSION=6.5
TARBALL=https://ftp.gnu.org/gnu/ncurses/ncurses-6.5.tar.gz
SHA256=136d91bc269a9a5785e5f9e980bc76ab57428f604ce3e5a5a90cebc767971cc6
BUILDSYS=autotools
# QNX's <sys/termio.h> and <termios.h> are mutually exclusive by design -- the
# header ends in "#error termio/termios incompatibility" when both are pulled
# in. ncurses includes sys/termio.h whenever configure finds it, so tell
# configure it is not there and let it use termios alone.
# ncurses links its shared libraries by invoking ld directly, so LDFLAGS must
# not carry compiler-driver syntax (ld rejects -Wl,-rpath-link) -- but configure's
# own link test runs through qcc, which rejects the bare ld spelling just as
# hard. Neither form works for both, so drop -rpath-link altogether: ncurses
# needs nothing from staging but libc, and transitive resolution now goes
# through the link sysroot that env.sh builds.
# The SDP library directory has to be named explicitly: ncurses links its
# shared libraries with a raw "ld" invocation that inherits only LDFLAGS, and
# it links -lregex -- which QNX keeps in $QNX_TARGET/<arch>/lib, a directory
# only the compiler driver knows about.
CONF_ENV=( ac_cv_header_sys_termio_h=no ac_cv_header_termio_h=no
           LDFLAGS="-L$STAGE$PREFIX/lib -L$QNX_TARGET/$QNX_ARCH/lib" )
# --without-progs: reset/tset use TAB3, a Linux termios delay constant QNX does
# not define. The target needs the library, not the terminal utilities.
CONF_ARGS_EXTRA=( --with-shared --without-debug --without-ada --without-tests --without-progs
                  --enable-widec --enable-pc-files --without-manpages
                  --with-pkg-config-libdir="$STAGE$PREFIX/lib/pkgconfig" )

# --enable-widec renames every library to the "w" variant, so a consumer asking
# for -lncurses or -ltinfo finds nothing here and the linker falls through to
# the SDP's own prebuilt ncurses (soname libncursesw.so.1 against our .so.6).
# Distributions solve this with compatibility symlinks; do the same.
do_install() {
  cd "$B" && make install DESTDIR="$STAGE" >/dev/null
  local d="$STAGE$PREFIX/lib"
  for n in ncurses form menu panel; do
    [ -e "$d/lib${n}w.so" ] && ln -sfn "lib${n}w.so" "$d/lib${n}.so"
    [ -e "$d/lib${n}w.a" ]  && ln -sfn "lib${n}w.a"  "$d/lib${n}.a"
  done
  # ncurses builds terminfo into libncursesw unless --with-termlib is used, so
  # -ltinfo has to resolve to the same library.
  [ -e "$d/libncursesw.so" ] && ln -sfn "libncursesw.so" "$d/libtinfo.so"
  [ -e "$d/libncursesw.a" ]  && ln -sfn "libncursesw.a"  "$d/libtinfo.a"
  for p in ncurses form menu panel tinfo; do
    [ -e "$d/pkgconfig/${p}w.pc" ] && ln -sfn "${p}w.pc" "$d/pkgconfig/${p}.pc"
  done
  find "$STAGE$PREFIX" -name '*.la' -delete
}
