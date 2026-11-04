# Samba: SMB/CIFS file sharing -- the client tools (smbclient, nmblookup,
# smbget, smbtree, rpcclient, net, ...) with libsmbclient, and the file
# server (smbd, nmbd) for a standalone server: shares, tdbsam users,
# SMB2/3 with signing and encryption (GnuTLS). Not built: the AD domain
# controller, AD membership (ads), winbind, LDAP, PAM, ACL/quota support,
# printing (CUPS) and the Python bindings -- each needs a Linux facility,
# a library not ported here or a desktop/directory service.
VERSION=4.25.1
TARBALL=https://download.samba.org/pub/samba/stable/samba-4.25.1.tar.gz
SHA256=7f367545bca69aa96897628f82f0f04d4b66b3bcb36cb978b9bb952c4b4c428d
DEPS="gnutls tdb libiconv icu zlib libxcrypt"
BUILDSYS=custom
# The build machine needs what a native Samba configure needs (gcc, perl,
# python3, flex, bison, the GnuTLS development files): see _smb_native.
#
# pidl needs Parse::Yapp on the build machine; it is fetched from CPAN
# (pinned) and installed into the native helper directory.
_YAPP_TB=https://cpan.metacpan.org/authors/id/W/WB/WBRASWELL/Parse-Yapp-1.21.tar.gz
_YAPP_SHA=3810e998308fba2e0f4f26043035032b027ce51ce5c8a52a8b8e340ca65f13e5
_SMB_NATIVE="$BUILD_DIR/native-samba-$VERSION"
_smb_yapp() {
  [ -x "$_SMB_NATIVE/yapp/bin/yapp" ] && return 0
  local f="$WS/dist/$(basename "$_YAPP_TB")"
  [ -f "$f" ] || curl -fsSL -o "$f" "$_YAPP_TB" || return 1
  echo "$_YAPP_SHA  $f" | sha256sum -c --status || { echo "!! checksum: $f"; return 1; }
  rm -rf "$_SMB_NATIVE/yapp-src" && mkdir -p "$_SMB_NATIVE/yapp-src" &&
  tar xf "$f" -C "$_SMB_NATIVE/yapp-src" --strip-components=1 && cd "$_SMB_NATIVE/yapp-src" &&
  env PATH=/usr/bin:/bin perl Makefile.PL INSTALL_BASE="$_SMB_NATIVE/yapp" && env PATH=/usr/bin:/bin make install
}
# Heimdal's asn1_compile and compile_et run while Samba builds; waf builds
# them with the target compiler. A native configure of the same source
# builds just those two (patches/0001 makes the cross build use them via
# SAMBA_NATIVE_HEIMDAL_TOOLS).
_smb_native() {
  [ -x "$_SMB_NATIVE/src/bin/default/third_party/heimdal_build/asn1_compile" ] &&
  [ -x "$_SMB_NATIVE/src/bin/default/third_party/heimdal_build/compile_et" ] && return 0
  rm -rf "$_SMB_NATIVE/src" && cp -a "$S" "$_SMB_NATIVE/src" && cd "$_SMB_NATIVE/src" &&
  env -i HOME="$HOME" PATH="/usr/bin:/bin:$_SMB_NATIVE/yapp/bin" PERL5LIB="$_SMB_NATIVE/yapp/lib/perl5" \
    ./configure --without-ad-dc --disable-python --without-ads --without-ldap --without-winbind \
      --without-pam --without-acl-support --without-quotas --without-systemd --without-libarchive \
      --without-gpgme --disable-cups --disable-iprint --disable-avahi --disable-glusterfs \
      --disable-cephfs --disable-cephrgw --without-json --without-ldb-lmdb --without-regedit \
      --disable-spotlight --bundled-libraries=ALL --without-cluster-support --without-utmp \
      --without-fam --without-dmapi --without-automount &&
  env -i HOME="$HOME" PYTHONHASHSEED=1 PATH="/usr/bin:/bin:$_SMB_NATIVE/yapp/bin" PERL5LIB="$_SMB_NATIVE/yapp/lib/perl5" \
    buildtools/bin/waf build --targets=asn1_compile,compile_et -j"$(nproc)"
}
# The cross build. Notes:
#  - configure questions that need a test program run are answered from
#    ports/samba/cross-answers-<arch>.txt, recorded on QNX targets (see
#    the comments there); waf builds in the source tree, so in a copy.
#  - staging headers as -isystem (through qcc's -Wc,): after Samba's and
#    Heimdal's own include directories (libmd's staged md5.h/sha.h would
#    shadow Heimdal's hcrypto ones), before the SDP's (its ICU 67 headers
#    would shadow the staged ICU).
#  - -lregex and -lfsnotify: regcomp() and inotify are not in QNX's libc
#    (hazards 2, 64).
#  - --enable-fhs: required for a /usr/local prefix; state under
#    /usr/local/var, configuration in /usr/local/etc/samba.
#  - tdb is the ported one (ports/tdb); talloc, tevent, ldb, popt and
#    cmocka are Samba's bundled private copies (lib/samba, *-private-samba).
#  - no gettext: libintl's printf replacements were not linked into
#    Heimdal's com_err; messages stay English.
#  - readline is not used: its 8.2 headers fail Samba's strict-prototypes
#    probe, so smbclient reads commands without line editing.
_smb_env() {
  env CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CFLAGS="-O2 -Wc,-isystem,$STAGE_SYSROOT$PREFIX/include" \
    LDFLAGS="-L$STAGE_SYSROOT$PREFIX/lib -Wl,-rpath-link,$STAGE_SYSROOT$PREFIX/lib -lsocket -lregex -lfsnotify" \
    PYTHONHASHSEED=1 PATH="$PATH:$_SMB_NATIVE/yapp/bin" PERL5LIB="$_SMB_NATIVE/yapp/lib/perl5" \
    SAMBA_NATIVE_HEIMDAL_TOOLS="$_SMB_NATIVE/src/bin/default/third_party/heimdal_build" "$@"
}
do_build() {
  mkdir -p "$_SMB_NATIVE" && ( _smb_yapp ) && ( _smb_native ) || return 1
  rm -rf "$B" && cp -a "$S" "$B" && cd "$B" &&
  cp "$WS/ports/samba/cross-answers-$QNX_ARCH.txt" "$B/cross-answers.txt" &&
  _smb_env ./configure --prefix="$PREFIX" --enable-fhs --sysconfdir="$PREFIX/etc" --localstatedir="$PREFIX/var" \
    --cross-compile --cross-answers="$B/cross-answers.txt" --hostcc=gcc \
    --disable-python --without-ad-dc --without-ads --without-ldap --without-winbind --without-pam \
    --without-acl-support --without-quotas --without-systemd --without-libarchive --without-gpgme \
    --disable-cups --disable-iprint --disable-avahi --disable-glusterfs --disable-cephfs --disable-cephrgw \
    --without-fam --without-automount --without-dmapi --without-utmp --without-json --without-regedit \
    --without-winexe --disable-spotlight --without-ldb-lmdb --without-libunwind --without-lttng \
    --without-cluster-support --without-kernel-keyring --without-systemd-userdb --disable-rpath \
    --disable-rpath-install --bundled-libraries='!tdb,talloc,tevent,ldb,popt,cmocka' \
    --with-shared-modules='!vfs_snapper' --with-libiconv="$STAGE_SYSROOT$PREFIX" --without-gettext &&
  _smb_env make -j"$(nproc)"
}
# The state directories Samba creates only one level of (it does not
# mkdir -p): empty ones are not packaged, so each gets a placeholder and a
# packaged samba starts with its compiled-in paths.
do_install() { cd "$B" && _smb_env make install DESTDIR="$STAGE" || return 1
  local d; for d in log/samba lib/samba lib/samba/private lock/samba cache/samba run/samba; do
    mkdir -p "$STAGE$PREFIX/var/$d" && : > "$STAGE$PREFIX/var/$d/.keep" || return 1; done
  chmod 700 "$STAGE$PREFIX/var/lib/samba/private"; }
