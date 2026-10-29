# xdg-utils: xdg-open and friends (shell scripts).
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.2.1
TARBALL=https://gitlab.freedesktop.org/xdg/xdg-utils/-/archive/v1.2.1/xdg-utils-v1.2.1.tar.gz
SHA256=f6b648c064464c2636884c05746e80428110a576f8daacf46ef2e554dcfdae75
BUILDSYS=autotools
CONF_ARGS_EXTRA=(  )
# The Makefiles only work in the source tree (the HTML/man pages are made from
# desc/*.xml by relative paths); build in a copy of it, scripts only.
BUILDSYS=custom
do_configure() { cp -a "$S/." "$B/" && cd "$B" && ./configure --prefix="$PREFIX"; }
# Each script embeds its --help/--manual text, which the Makefile makes with
# "xmlto txt" from desc/*.xml. xmlto (with the DocBook XSL and a text browser)
# is rarely on a build machine; refentry2txt.py renders the same text layout
# with Python alone.
do_build() { make -C "$B/scripts" scripts XMLTO="python3 $RECIPE_DIR/xdg-utils/refentry2txt.py"; }
do_install() { make -C "$B/scripts" DESTDIR="$STAGE" install; }
