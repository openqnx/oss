VERSION=3.5.1; GIT=https://github.com/openssl/openssl.git; REF=openssl-3.5.1
BUILDSYS=custom

do_configure() {
  cp "$RECIPE_DIR/openssl/files/50-qnx.conf" "$S/Configurations/50-qnx.conf"
  cd "$B"
  # no-asm: the perlasm backends emit GNU-as syntax that qcc's assembler driver
  # does not accept for every module; correctness first, revisit for perf later.
  perl "$S/Configure" "qnx-$QNX_ARCH" \
      --prefix="$PREFIX" --openssldir="$PREFIX/ssl" \
      --libdir=lib \
      shared no-asm no-tests no-docs no-apps threads \
      -D_QNX_SOURCE
}
do_build()   { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install_sw; }
