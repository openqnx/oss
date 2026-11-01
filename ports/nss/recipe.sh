# NSS (Network Security Services): TLS, PKCS#11, certificates, and the
# certutil/pk12util/modutil tools. Built with NSS's own gyp + ninja build
# (build.sh), the build qnx-ports' QNX changes (patches/0001) are made for.
VERSION=3.131.1
TARBALL=https://ftp.mozilla.org/pub/security/nss/releases/NSS_3_131_1_RTM/src/nss-3.131.1.tar.gz
SHA256=b1dbc1d9e7f3f8455dfb6d58ec8f7229d8a8450c629a9e69ef2ef6081f359056
BUILDSYS=custom
SUBDIR=nss
DEPS="nspr sqlite zlib"
# gyp is not installed on the build machine: build.sh runs gyp-next, pinned
# here, from the source tree.
GYP_NEXT_GIT=https://github.com/nodejs/gyp-next.git; GYP_NEXT_REF=v0.22.4
do_patch() { git clone -q --depth 1 --branch "$GYP_NEXT_REF" "$GYP_NEXT_GIT" "$SRCROOT/gyp-next"; }
# build.sh writes out/ and ../dist next to the sources: build a copy per
# architecture so the two never share objects.
do_configure() { mkdir -p "$B/nss" && cp -a "$S/." "$B/nss/"; }
do_build() {
  cd "$B/nss" || return 1
  local t; case "$arch" in x86_64) t=x64;; aarch64le) t=aarch64;; esac
  # The cross-GCC driver, not qcc: gyp's ninja rules pass -MMD -MF <file>,
  # which qcc misreads as a second input file (porting-hazards 18/21).
  # CC_host: the build also compiles helpers for the build machine.
  # -DOS=qnx selects qnx-ports' gyp conditions (gyp itself would say
  # "linux"). -Dsign_libs=0: shlibsign would have to run the target's
  # softoken; its .chk files are only for FIPS mode, which stays disabled.
  CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc" CXX="$QNX_HOST/usr/bin/$QNX_TRIPLE-g++" \
  AR="$QNX_HOST/usr/bin/$QNX_TRIPLE-ar" CC_host=gcc CXX_host=g++ AR_host=ar \
  CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib" \
  GYP="python3 $SRCROOT/gyp-next/gyp_main.py" \
  ./build.sh -v -j "$(nproc)" --opt -t "$t" --disable-tests \
    --with-nspr="$STAGE$PREFIX/include/nspr:$STAGE$PREFIX/lib" \
    --system-sqlite -Duse_system_zlib=1 -DOS=qnx -Dsign_libs=0
}
# build.sh has no install step: copy the libraries, public headers and tools
# out of dist/, and write nss.pc / nss-config for the target's paths.
do_install() {
  local d="$B/dist" r="$B/dist/Release" v="$VERSION" nsprv
  mkdir -p "$STAGE$PREFIX/lib/pkgconfig" "$STAGE$PREFIX/include/nss" "$STAGE$PREFIX/bin"
  cp -a "$r"/lib/*.so "$STAGE$PREFIX/lib/" || return 1
  cp -a "$d"/public/nss/. "$STAGE$PREFIX/include/nss/" || return 1
  # The classic NSS tools. Left out: shlibsign (FIPS .chk files only),
  # nss-config (written below), and the developer tools with generic names
  # (nss, dbtool, hw-support, validation).
  local p
  for p in certutil cmsutil crlutil modutil nssdefaults pk12util pwdecrypt signtool signver ssltap symkeyutil; do
    install -m755 "$r/bin/$p" "$STAGE$PREFIX/bin/" || return 1; done
  nsprv=$(sed -n 's/^Version: *//p' "$STAGE_SYSROOT$PREFIX/lib/pkgconfig/nspr.pc")
  sed -e "s|%prefix%|$PREFIX|; s|%exec_prefix%|\${prefix}|; s|%libdir%|\${exec_prefix}/lib|" \
      -e "s|%includedir%|\${prefix}/include/nss|; s|%NSS_VERSION%|$v|; s|%NSPR_VERSION%|$nsprv|" \
      "$S/pkg/pkg-config/nss.pc.in" > "$STAGE$PREFIX/lib/pkgconfig/nss.pc"
  sed -e "s|@prefix@|$PREFIX|; s|@MOD_MAJOR_VERSION@|${v%%.*}|" \
      -e "s|@MOD_MINOR_VERSION@|$(echo $v | cut -d. -f2)|; s|@MOD_PATCH_VERSION@|$(echo $v | cut -d. -f3)|" \
      "$S/pkg/pkg-config/nss-config.in" > "$STAGE$PREFIX/bin/nss-config"
  chmod 755 "$STAGE$PREFIX/bin/nss-config"
}
