VERSION=2.3.8
# libidn2 releases live under gnu/libidn/, not gnu/libidn2/.
TARBALL=https://ftp.gnu.org/gnu/libidn/libidn2-2.3.8.tar.gz
SHA256=f557911bf6171621e1f72ff35f5b1825bb35b52ed45325dcdee931e5d3c0787a
BUILDSYS=autotools
# Also linked at run time (found by scripts/gen-dependency-index): gettext
DEPS="libunistring gettext"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc --disable-gcc-warnings )
