# hwdata: the hardware identification databases -- pci.ids, usb.ids,
# pnp.ids, the IEEE OUI/IAB registries -- in share/hwdata, as read by
# lspci/lsusb-style tools and libraries (and hwdata.pc to find them).
# Data, the same for every architecture.
VERSION=0.413
TARBALL=https://github.com/vcrhonek/hwdata/archive/refs/tags/v0.413.tar.gz
SHA256=5a71ab7a48c668ef912fcfdbec046949087019f8dd7fb967445d36fbeb778350
BUILDSYS=custom
# Its own configure (not autoconf) writes Makefile.inc. The kernel-module
# blacklist (lib/modprobe.d) is a Linux modprobe file: left out.
do_build() { rm -rf "$B" && cp -a "$S" "$B" && cd "$B" && ./configure --prefix="$PREFIX" --disable-blacklist; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
