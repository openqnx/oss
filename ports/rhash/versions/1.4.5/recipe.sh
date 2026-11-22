# RHash: computes and verifies hash sums and magnet links (CRC32, MD5,
# SHA-1/2/3, BLAKE2/3, Tiger, TTH, BTIH, ED2K, GOST, ...) -- the rhash
# program and the LibRHash library. Version as in qnx-ports' aports.
VERSION=1.4.5
TARBALL=https://github.com/rhash/RHash/archive/refs/tags/v1.4.5.tar.gz
SHA256=6db837e7bbaa7c72c5fd43ca5af04b1d370c5ce32367b9f6a1f7b49b2338c09a
DEPS=""
# Its own configure (not Autoconf), building in the source tree: configured
# and built in a copy (src/ stays clean for scripts/mkpatch).
# --target <cpu>-pc-qnx: RHash takes the OS from the triple's third field,
# which in QNX's own triple (x86_64-pc-nto-qnx8.0.0) is "nto"; "qnx" is the
# name it knows (it then writes a POSIX make file). No OpenSSL (its optional faster MD5/SHA, and the
# hashes are all built in) and no gettext.
# --cc: the SDP's cross gcc itself, not qcc: configure identifies the
# compiler from the last line of "$CC -v", which for qcc is not gcc's
# ("compiler not found"). That driver links for QNX on its own.
BUILDSYS=custom
do_configure() {
  rm -rf "$B" && mkdir -p "$B" && cp -a "$S/." "$B/" || return 1
  cd "$B" && ./configure --prefix="$PREFIX" --target="$QNX_CPU-pc-qnx" \
    --cc="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc" --ar="$QNX_HOST/usr/bin/$QNX_TRIPLE-ar" \
    --extra-cflags="-D_QNX_SOURCE -O2" \
    --disable-openssl --disable-gettext --enable-lib-shared --disable-lib-static
}
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() {
  make -C "$B" DESTDIR="$STAGE" install install-lib-headers install-lib-so-link install-pkg-config
}
