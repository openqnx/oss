# GNU tar. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.35
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="libiconv"
TARBALL=https://ftp.gnu.org/gnu/tar/tar-1.35.tar.xz
SHA256=4d62ff37342ec7aed748535323930c7cf94acf71c3591882b26a7ea50f3edc16
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls )
# The libiconv component's <iconv.h> renames iconv_open to libiconv_open, so
# link that library explicitly.
CONF_ENV=( LIBS="-liconv" )
