# Upstream toybox (landley.net), as one multicall binary with no applet
# links, in a prefix of its own: /usr/local/opt/toybox/bin/toybox. QNX's
# base image ships its own toybox, whose names this must not take over;
# run an applet as "toybox <applet>" (or link it yourself).
VERSION=0.8.15
TARBALL=https://landley.net/toybox/downloads/toybox-0.8.15.tar.gz
SHA256=44e2bab43dfe2c171743297631ce558c6d9ed38b87a66fd751cef617574b375e
BUILDSYS=custom
# The applets: miniconfig (macOS's set, less the ones that need a
# mount table, utmpx or Linux /proc); kconfig itself runs on the build
# machine (HOSTCC). The QNX patches add a QNX branch to the portability
# layer (statvfs for statfs, no xattr, no Linux syscall numbers, inotify via
# libfsnotify), and replace dirent d_type and the locale merging.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && \
  KCONFIG_ALLCONFIG="$RECIPE_DIR/toybox/miniconfig" make allnoconfig HOSTCC=gcc; }
# The cross-GCC driver (toybox's make.sh composes its own command lines);
# regex is in libregex, sockets in libsocket, inotify in libfsnotify. No zlib/libcrypto (the
# config leaves TOYBOX_LIBZ/LIBCRYPTO off: the SDP's copies would be found).
do_build() { cd "$B" && CROSS_COMPILE="$QNX_HOST/usr/bin/$QNX_TRIPLE-" CC=gcc HOSTCC=gcc \
  STRIP="$QNX_HOST/usr/bin/nto${QNX_CPU}-strip" CFLAGS="-D_QNX_SOURCE" LDFLAGS="-lregex -lsocket -lfsnotify" \
  CPUS=6 V=1 scripts/make.sh; }
do_install() { install -Dm755 "$B/toybox" "$STAGE$PREFIX/opt/toybox/bin/toybox"; }
