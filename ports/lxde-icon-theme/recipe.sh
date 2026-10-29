# The LXDE (nuoveXT) icon theme (data only). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=0.5.2
TARBALL=https://github.com/lxde/lxde-icon-theme/archive/refs/tags/0.5.2.tar.gz
SHA256=5b71da247ba25ddcd991a3a184ca5ac92f40b7676766e1e59437067a20f7ecf7
BUILDSYS=autotools
CONF_ARGS_EXTRA=(  )
# Icon data only, installed by its Makefiles from the source tree; build in a
# copy of it.
BUILDSYS=custom
do_configure() { cp -a "$S/." "$B/" && cd "$B" && autoreconf -fi && ./configure --prefix="$PREFIX"; }
do_build() { make -C "$B"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
