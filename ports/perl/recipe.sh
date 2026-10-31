VERSION=5.40.2
# Also linked at run time (found by scripts/gen-dependency-index): libxcrypt
DEPS="libxcrypt"
TARBALL=https://www.cpan.org/src/5.0/perl-5.40.2.tar.gz
SHA256=10d4647cfbb543a7f9ae3e5f6851ec49305232ea7621aed24c7cfbb0bef4b70d
BUILDSYS=custom
# Perl cannot cross compile with its own Configure (it runs probes on the
# target). perl-cross replaces Configure with a cross-capable one: its tree is
# unpacked over the perl source (do_fetch). perl-cross knows QNX (osname
# "nto", cnf/hints/nto) since 1.6.2 -- the change qnx-ports carried for 1.6.1
# went upstream -- so only the perl patch in patches/ remains. This is the perl
# version the QNX Everywhere 8.0.4r1 image ships; it also covers qnx-ports'
# perl5 port.
PERLCROSS_URL=https://github.com/arsv/perl-cross/releases/download/1.6.2/perl-cross-1.6.2.tar.gz
PERLCROSS_SHA256=131f7496152ee32067dbac2bc9b44b2f582fc778140e545701b3b2faee782f1d
do_fetch() {
  local tb="$WS/dist/perl-5.40.2.tar.gz" pc="$WS/dist/perl-cross-1.6.2.tar.gz"
  mkdir -p "$WS/dist" "$SRCROOT"
  [ -f "$tb" ] || curl -fsSL -o "$tb" "$TARBALL" || return 1
  [ -f "$pc" ] || curl -fsSL -o "$pc" "$PERLCROSS_URL" || return 1
  echo "$SHA256  $tb" | sha256sum -c --status || { echo "perl checksum mismatch"; return 1; }
  echo "$PERLCROSS_SHA256  $pc" | sha256sum -c --status || { echo "perl-cross checksum mismatch"; return 1; }
  # perl first, perl-cross over it; patches/ then apply to the combined tree.
  tar xf "$tb" -C "$SRCROOT" --strip-components=1 && tar xf "$pc" -C "$SRCROOT" --strip-components=1
}
# The preprocessor is the cross-GCC driver, not qcc: for qcc, -P means
# "preprocess to a file" (try.i), not "no line markers". perl-cross's probes
# run "$cpp -P" and read stdout, and Errno_pm.PL adds -P for GCC >= 5, so with
# qcc every signal-name probe and every errno value came back empty: perl had
# no %SIG names ("No such signal: SIGHUP") and an empty %! (hazard 131).
# Errno_pm.PL's first pass needs line markers, hence cpprun/cppstdin
# without -P. gccversion: perl-cross asks qcc and gets "0.0", so Errno_pm.PL
# did not add -P to its second pass either, and GCC split each
# '"EEXIST" [[17]]' line with line markers -- an empty %! again.
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && \
  ./configure --target="$QNX_TRIPLE" --prefix="$PREFIX" -Dcc="qcc -V$QCC_TARGET" -Dld="qcc -V$QCC_TARGET" \
    -Dar="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" -Dccflags="-D_QNX_SOURCE" -Duseshrplib -Dusethreads \
    -Dusrinc="$QNX_TARGET_SDP/usr/include" -Dincpth="$QNX_TARGET_SDP/usr/include" \
    -Dcpp="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E -P" -Dgccversion="$("$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc" -dumpversion)" \
    -Dcpprun="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" -Dcppstdin="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    -Dlibpth="$QNX_TARGET/$QNX_ARCH/lib $QNX_TARGET/$QNX_ARCH/usr/lib"
}
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
