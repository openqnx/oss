# GNU m4. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.4.19
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="libiconv"
TARBALL=https://ftp.gnu.org/gnu/m4/m4-1.4.19.tar.xz
SHA256=63aede5c6d33b6d9b13511cd0be2cac046f2e70fd0a07aa9573a04a82783af96
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls )
