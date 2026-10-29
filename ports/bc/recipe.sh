# GNU bc, the arbitrary-precision calculator. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.08.2
TARBALL=https://ftp.gnu.org/gnu/bc/bc-1.08.2.tar.gz
SHA256=ae470fec429775653e042015edc928d07c8c3b2fc59765172a330d3d87785f86
DEPS="readline"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --with-readline )
