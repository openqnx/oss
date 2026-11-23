# BusyBox with qnx-ports' QNX support (patches/0001: the fork's QNX commit,
# made on 1.36.1), built with the applet configuration of the QNX Everywhere 8.0.4r1
# image (busybox.config, from qnx-ports/aports core/busybox).
VERSION=1.38.0; GIT=https://github.com/vda-linux/busybox_mirror.git; REF=1_38_0
BUILDSYS=custom
_bbmake() { make -C "$B" -j"$(nproc)" CROSS_COMPILE="$QNX_HOST/usr/bin/nto${QNX_CPU}-" CC="qcc -V$QCC_TARGET" \
  HOSTCC=gcc HOSTLDFLAGS="" LDFLAGS="-Wl,-z,stack-size=4194304" \
  CONFIG_EXTRA_CFLAGS="-D_QNX_SOURCE -DBB_GLOBAL_CONST=''" CONFIG_EXTRA_LDLIBS='"regex socket"' "$@"; }
# CONFIG_EXTRA_LDLIBS carries its own double quotes, as a .config string
# does: since 1.38 the link rule passes it to scripts/trylink unquoted, and a
# bare "regex socket" split into two arguments and dropped -lsocket.
# oldconfig asks about options the saved .config does not mention; "yes" takes
# the defaults. yes ends on SIGPIPE when make is done, which under the
# driver's pipefail failed the whole step, hence the "|| :".
# Applets newer than busybox.config that cannot build on QNX are switched off
# before oldconfig: getfattr (needs <sys/xattr.h>), lsblk (needs <mntent.h>
# and Linux /sys/block).
_BB_OFF="GETFATTR LSBLK"
do_configure() { cp -a "$S/." "$B/" && cp "$RECIPE_DIR/busybox/busybox.config" "$B/.config" &&
  for o in $_BB_OFF; do echo "# CONFIG_$o is not set" >> "$B/.config"; done &&
  { yes "" || :; } | _bbmake oldconfig >/dev/null; }
do_build() { _bbmake busybox; }
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/busybox" "$STAGE$PREFIX/bin/"; }
