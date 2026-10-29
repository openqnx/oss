# fakeroot (run as fake root for packaging). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.37.1.1
TARBALL=https://deb.debian.org/debian/pool/main/f/fakeroot/fakeroot_1.37.1.1.orig.tar.gz
SHA256=86b0b75bf319ca42e525c098675b6ed10a06b76e69ec9ccf20ef5e03883b3a14
BUILDSYS=autotools
# configure cannot test which IPC works when cross compiling; fakeroot uses
# its TCP transport on QNX.
CONF_ARGS_EXTRA=( --disable-static --with-ipc=tcp )
