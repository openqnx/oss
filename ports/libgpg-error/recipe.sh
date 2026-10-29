VERSION=1.50
# Also linked at run time (found by scripts/gen-dependency-index): gettext
DEPS="gettext"
TARBALL=https://gnupg.org/ftp/gcrypt/libgpg-error/libgpg-error-1.50.tar.bz2
SHA256=69405349e0a633e444a28c5b35ce8f14484684518a508dc48a089992fe93e20a
BUILDSYS=autotools
# libgpg-error needs a syscfg/lock-obj-pub.<triplet>.h describing the target's
# pthread_mutex_t layout, and ships none for QNX. The two under files/ were
# produced by building the upstream generator (src/gen-posix-lock-obj.c) for QNX
# and running it on the real targets -- a Raspberry Pi 5 for aarch64le and the
# QEMU VM for x86_64. QNX's pthread_mutex_t is 8 bytes on both and the two
# generated headers are byte-identical, so a CPU-less lock-obj-pub.nto-qnx.h is
# shipped as well: mkheader falls back to that spelling for the x86_64 triplet.
do_patch() {
  cp "$RECIPE_DIR/libgpg-error/files/lock-obj-pub."*.h "$SRCROOT/src/syscfg/"
  ls "$SRCROOT/src/syscfg/" | grep nto-qnx
}

CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc --disable-tests
                  --disable-languages )
