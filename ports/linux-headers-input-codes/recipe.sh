# linux-headers-input-codes: the Linux input event codes (KEY_*, BTN_*, ...)
# as one header, linux/input-event-codes.h -- defines only, the shared
# vocabulary of Wayland input (wlroots' QNX Screen backend maps Screen events
# to them, Wayland clients read them). QNX ships it under this name too.
# GPL-2.0 WITH Linux-syscall-note: usable from any program.
VERSION=6.12
TARBALL="https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/plain/include/uapi/linux/input-event-codes.h?h=v6.12"
FETCH_NAME=input-event-codes-6.12.h
SHA256=f9889a73d7d4f7e303d2d346a3622657169c5e8d7d354cb7a030ff47a7fdfdee
DEPS=""
BUILDSYS=custom
do_fetch() {
  local a="$WS/dist/$FETCH_NAME"; mkdir -p "$WS/dist" "$SRCROOT"
  [ -f "$a" ] || curl -fsSL -o "$a" "$TARBALL" || return 1
  echo "$SHA256  $a" | sha256sum -c --status || { echo "checksum mismatch: $a"; rm -f "$a"; return 1; }
  cp "$a" "$SRCROOT/input-event-codes.h"
}
do_configure() { :; }
do_build() { :; }
do_install() { install -Dm644 "$S/input-event-codes.h" "$STAGE$PREFIX/include/linux/input-event-codes.h"; }
