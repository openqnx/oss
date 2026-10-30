# xcb-util: XCB utility library (atoms, events, aux helpers).
VERSION=0.4.1
TARBALL=https://xorg.freedesktop.org/releases/individual/xcb/xcb-util-0.4.1.tar.xz
SHA256=5abe3bbbd8e54f0fa3ec945291b7e8fa8cfd3cccc43718f8758430f94126e512
DEPS="libxcb"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
