# itstool (XML translation tool; Python). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.0.7
TARBALL=https://files.itstool.org/itstool/itstool-2.0.7.tar.bz2
SHA256=6b9a7cd29a12bb95598f5750e8763cee78836a1a207f85b74d8b3275b27e87ca
DEPS="python3 libxml2"
BUILDSYS=autotools
CONF_ARGS_EXTRA=(  )
# The QNX patch changes configure.ac; regenerate configure from it.
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
