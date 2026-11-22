# GNU m4. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.4.21
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="libiconv"
TARBALL=https://ftp.gnu.org/gnu/m4/m4-1.4.21.tar.xz
SHA256=f25c6ab51548a73a75558742fb031e0625d6485fe5f9155949d6486a2408ab66
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls )
