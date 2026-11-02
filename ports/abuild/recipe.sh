# abuild: Alpine's APK package build tool (shell scripts plus small C
# helpers) "adapted for QNX": upstream 3.17.0 (GitHub mirror of
# gitlab.alpinelinux.org) with qnx-ports' QNX commits (patches/, made for
# 3.16.0 -- the release QNX's aports builds as 3.16.0_p5 -- and refreshed for
# 3.17.0; the image ships 3.15.0); the scripts expect a POSIX shell, apk-tools
# and the usual build utilities on the target.
VERSION=3.17.0; GIT=https://github.com/alpinelinux/abuild.git; REF=3.17.0
DEPS="openssl zlib apk-tools"
BUILDSYS=custom
# The man pages are made with scdoc, which the build host does not have;
# SCDOC=true leaves them empty, and they are not installed.
_mk() { make -C "$B" SCDOC=true CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib" prefix="$PREFIX" sysconfdir="$PREFIX/etc" "$@"; }
do_configure() { cp -a "$S/." "$B/"; }
# 3.17.0 dropped buildlab and added abuild-rm (C; abuild's clean steps call it
# to remove trees, read-only directories included).
do_build() { _mk abuild-tar abuild-gzsplit abuild-sudo abuild-rm abuild abuild-keygen abuild-sign newapkbuild abump apkgrel checkapk functions.sh; }
do_install() { _mk DESTDIR="$STAGE" install && rm -rf "$STAGE$PREFIX/share/man"; }
