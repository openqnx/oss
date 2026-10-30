# libxshmfence: shared-memory fences, the synchronisation X servers and
# DRI3 clients share (futex-like, over a shared file).
VERSION=1.3.3
TARBALL=https://xorg.freedesktop.org/releases/individual/lib/libxshmfence-1.3.3.tar.xz
SHA256=d4a4df096aba96fea02c029ee3a44e11a47eb7f7213c1a729be83e85ec3fde10
DEPS="xorgproto"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
