# libsm: X session management (clients saving and restoring their state with a session manager).
VERSION=1.2.6
TARBALL=https://xorg.freedesktop.org/releases/individual/lib/libSM-1.2.6.tar.xz
SHA256=be7c0abdb15cbfd29ac62573c1c82e877f9d4047ad15321e7ea97d1e43d835be
DEPS="libice xorgproto xtrans"
BUILDSYS=autotools
# libuuid is optional (client ids); without it libSM makes its own ids.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --without-libuuid --without-xmlto --without-fop --disable-docs )
