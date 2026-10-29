VERSION=2.3.7
# libidn2 releases live under gnu/libidn/, not gnu/libidn2/.
TARBALL=https://ftp.gnu.org/gnu/libidn/libidn2-2.3.7.tar.gz
SHA256=4c21a791b610b9519b9d0e12b8097bf2f359b12f8dd92647611a929e6bfd7d64
BUILDSYS=autotools
# Also linked at run time (found by scripts/gen-dependency-index): gettext
DEPS="libunistring gettext"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc --disable-gcc-warnings )
