# GNU Emacs 30.2, terminal build (no X/GTK), with the QNX changes of QNX's
# aports (patches/). Version as in the QNX Everywhere 8.0.4r1 image.
#
# Emacs byte-compiles its Lisp and dumps its own image while building, which
# needs a running Emacs: a native build for the build host is made first and
# used for that (EMACS / BOOTSTRAPEMACS), then the target build reuses its
# compiled Lisp.
VERSION=30.2
TARBALL=https://ftp.gnu.org/gnu/emacs/emacs-30.2.tar.xz
SHA256=b3f36f18a6dd2715713370166257de2fae01f9d38cfe878ced9b1e6ded5befd9
# Also linked at run time (found by scripts/gen-dependency-index): dbus lcms2 sqlite tree-sitter
DEPS="ncurses gnutls libxml2 zlib dbus lcms2 sqlite tree-sitter"
BUILDSYS=custom
do_configure() {
  local H="$BUILD_DIR/host/emacs-$VERSION"
  if [ ! -x "$H/src/emacs" ]; then
    mkdir -p "$H" && ( cd "$H" && env -u CC -u CXX -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR \
      "$S/configure" --without-x --without-sound --with-native-compilation=no --without-gnutls --without-xml2 \
      --without-all --with-modules=no && make -j"$(nproc)" ) || return 1
  fi
  cd "$B" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CPPFLAGS="-I$STAGE$PREFIX/include" LDFLAGS="-L$STAGE$PREFIX/lib -lsocket" \
  "$S/configure" --host="$QNX_TRIPLE" --build=x86_64-pc-linux-gnu --prefix="$PREFIX" --without-x --without-sound \
    --without-file-notification --with-native-compilation=no --with-dumping=none --with-pdumper=no \
    --with-gnutls --with-xml2 --with-modules=no
}
# Emacs runs what it builds: lib-src's make-docfile (the DOC file, globals.h)
# and Emacs itself (byte-compiling, generated Lisp). Those come from the build
# host's Emacs made above -- libsrc= for the lib-src tools, EMACS= for Emacs;
# with --with-dumping=none there is no dump to make with the QNX binary.
_hostemacs() { echo "$BUILD_DIR/host/emacs-$VERSION"; }
# timerfd_create() is in QNX's libtimerfd, not libc. configure finds it (via
# other libraries) but temacs is linked with LIBS_SYSTEM, which the aports
# configure patch sets to -lsocket alone: "DSO missing from command line".
# "make all" is actual-all followed by a sanity check that runs the new
# emacs -- a QNX binary. Build actual-all; the check belongs on the target.
# install depends on actual-all too, so it gets the same host tools. EMACS
# means two things: the program lisp/ runs, and at the top level the name to
# install the binary as. Pass the top level's own value ("emacs") and let PATH
# resolve it to the host build -- an absolute path there became the name of
# the installed symlink.
_emake() { local H; H=$(_hostemacs)
  mkdir -p "$B/hostbin" && ln -sf "$H/src/emacs" "$B/hostbin/emacs" && \
  PATH="$B/hostbin:$PATH" make -C "$B" -j"$(nproc)" libsrc="$H/lib-src" LIBS_SYSTEM="-lsocket -ltimerfd" \
    EMACS=emacs BOOTSTRAPEMACS="$H/src/bootstrap-emacs" "$@"; }
do_build()   { _emake actual-all; }
do_install() { _emake DESTDIR="$STAGE" install; }
