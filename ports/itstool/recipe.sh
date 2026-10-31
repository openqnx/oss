# itstool (XML translation tool; Python). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.0.7
TARBALL=https://files.itstool.org/itstool/itstool-2.0.7.tar.bz2
SHA256=6b9a7cd29a12bb95598f5750e8763cee78836a1a207f85b74d8b3275b27e87ca
DEPS="python3 libxml2"
BUILDSYS=autotools
CONF_ARGS_EXTRA=(  )
# The QNX patch changes configure.ac; regenerate configure from it.
do_patch() { cd "$SRCROOT" && autoreconf -fi; }

# itstool is a Python script that imports libxml2's bindings (libxml2.py,
# libxml2mod.so in site-packages): no DT_NEEDED says so.
RUNTIME_DEPS="python3 libxml2"
# itstool's #! line is the build machine's python3 (configure's PYTHON);
# the target's python3 is $PREFIX/bin/python3 (hazard 105).
do_install() {
  make -C "$B" DESTDIR="$STAGE" install
  sed -i "1s|^#!.*|#!$PREFIX/bin/python3 -s|" "$STAGE$PREFIX/bin/itstool"
}
