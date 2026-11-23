# BusyBox 1.36.1 with qnx-ports' QNX support (patches/: the fork's QNX
# commit), built with the applet configuration of the QNX Everywhere 8.0.4r1
# image (busybox.config, from qnx-ports/aports core/busybox).
VERSION=1.36.1; GIT=https://github.com/vda-linux/busybox_mirror.git; REF=1_36_1
BUILDSYS=custom
_bbmake() { make -C "$B" -j"$(nproc)" CROSS_COMPILE="$QNX_HOST/usr/bin/nto${QNX_CPU}-" CC="qcc -V$QCC_TARGET" \
  HOSTCC=gcc HOSTLDFLAGS="" LDFLAGS="-Wl,-z,stack-size=4194304" \
  CONFIG_EXTRA_CFLAGS="-D_QNX_SOURCE -DBB_GLOBAL_CONST=''" CONFIG_EXTRA_LDLIBS="regex socket" "$@"; }
# oldconfig asks about options the saved .config does not mention; "yes" takes
# the defaults. yes ends on SIGPIPE when make is done, which under the
# driver's pipefail failed the whole step, hence the "|| :".
do_configure() { cp -a "$S/." "$B/" && cp "$RECIPE_DIR/busybox/busybox.config" "$B/.config" && { yes "" || :; } | _bbmake oldconfig >/dev/null; }
do_build() { _bbmake busybox; }
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/busybox" "$STAGE$PREFIX/bin/"; }
