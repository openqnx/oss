# man-db, the man page reader. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.13.1
TARBALL=https://download.savannah.gnu.org/releases/man-db/man-db-2.13.1.tar.xz
SHA256=8afebb6f7eb6bb8542929458841f5c7e6f240e30c86358c1fbcefbea076c87d9
# Also linked at run time (found by scripts/gen-dependency-index): libiconv zlib
DEPS="libpipeline gdbm groff libiconv zlib"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --disable-setuid --without-libseccomp --disable-cache-owner --with-systemdtmpfilesdir=no --with-systemdsystemunitdir=no )
# The "manual" directory typesets man-db's user guide with groff's -me macros
# on the build host, where they are often not installed (Debian's groff-base
# lacks them). The guide is not needed to run man; leave that directory out of
# the generated top-level Makefile. (Not with SUBDIRS=... on the make command
# line: that reaches every sub-make too, and gl/lib then looks for "docs".)
_no_manual() { sed -i '/^SUBDIRS = /s/ manual / /' "$B/Makefile"; }
do_build()   { _no_manual && make -C "$B" -j"$(nproc)"; }
do_install() { _no_manual && make -C "$B" DESTDIR="$STAGE" install; }
