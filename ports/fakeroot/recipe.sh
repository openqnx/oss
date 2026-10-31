# fakeroot (run as fake root for packaging). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.37.1.1
TARBALL=https://deb.debian.org/debian/pool/main/f/fakeroot/fakeroot_1.37.1.1.orig.tar.gz
SHA256=86b0b75bf319ca42e525c098675b6ed10a06b76e69ec9ccf20ef5e03883b3a14
BUILDSYS=autotools
# configure cannot test which IPC works when cross compiling; fakeroot uses
# its TCP transport on QNX.
CONF_ARGS_EXTRA=( --disable-static --with-ipc=tcp )
# The wrapper preloads FAKEROOT_LIB=libfakeroot.so, the unversioned name that
# qnxpkg puts in fakeroot-dev (it is the link-time symlink); the runtime
# package carries only libfakeroot-0.so (libtool -release 0), so `fakeroot`
# failed on a target with "preload library not found". Point it at the real
# file. Its shebang is configure's $SHELL -- the build host's /bin/bash, which
# a QNX target does not have; the script is POSIX sh (Debian runs it so).
do_install() {
  make -C "$B" DESTDIR="$STAGE" install
  sed -i -e 's|^FAKEROOT_LIB=libfakeroot\.so$|FAKEROOT_LIB=libfakeroot-0.so|' \
         -e '1s|^#!.*|#!/bin/sh|' "$STAGE$PREFIX/bin/fakeroot"
  grep -q '^FAKEROOT_LIB=libfakeroot-0.so$' "$STAGE$PREFIX/bin/fakeroot"
}
