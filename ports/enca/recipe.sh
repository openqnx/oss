VERSION=1.19; GIT=https://github.com/nijel/enca.git; REF=1.19
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-external )
# tools/make_hash is compiled and then *run* to generate encodings.h. Built for
# the target it is a QNX binary, and the build dies in the binfmt handler
# ("Could not open /usr/lib/ldqnx-64.so.2"). Build a native copy first and make
# the target one a wrapper around it -- the generated header is
# architecture-independent.
do_patch() {
  cd "$SRCROOT" && autoreconf -fi
}
do_build() {
  cc -O1 -o "$B/make_hash_native" "$S/tools/make_hash.c" -lm
  make -C "$B" -j"$(nproc)" || true
  # Replace the cross-built generator with the native one and resume.
  install -m 755 "$B/make_hash_native" "$B/tools/make_hash"
  make -C "$B" -j"$(nproc)"
}
