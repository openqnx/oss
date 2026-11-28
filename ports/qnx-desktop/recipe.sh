# qnx-desktop: the whole desktop with one command, "apk add qnx-desktop" --
# a session like Raspberry Pi OS' on QNX Screen, from the ported components.
# Like rpd-wayland-all there, it is only the sum of the groups below, each a
# meta package of its own that can be installed alone (a board short of space
# takes qnx-desktop-core and qnx-desktop-theme: the session and its look):
#   qnx-desktop-core          the session: labwc, Xfce panel/session/settings,
#                             Thunar, terminal, D-Bus; the boot entry
#                             qnx-desktop-start
#   qnx-desktop-theme         icon themes, fonts (CJK, emoji)
#   qnx-desktop-preferences   printing, service discovery, time
#   qnx-desktop-utilities     editor, image viewer, calculator, archives
#   qnx-desktop-applications  web browser, media players
#   qnx-desktop-developer     compiler, CMake, Python, Git, GDB, Node.js
#   qnx-desktop-graphics      GStreamer plugin sets, FFmpeg, Mesa
#   qnx-desktop-extras        desktop in a browser, VNC viewer, SSH, sudo
# What QNX itself must provide (Screen with the board's graphics support,
# input, network, a writable /data) is not a package: the board image carries it.
#
# To add when written (not in ports/ yet):
#   qnx-desktop-pi            board tools: vcgencmd/pinctrl counterparts, a
#                             configuration tool, panel temperature/voltage
VERSION=1.0
DEPS="qnx-desktop-core qnx-desktop-theme qnx-desktop-preferences qnx-desktop-utilities qnx-desktop-applications qnx-desktop-developer qnx-desktop-graphics qnx-desktop-extras"
RUNTIME_DEPS="qnx-desktop-core qnx-desktop-theme qnx-desktop-preferences qnx-desktop-utilities qnx-desktop-applications qnx-desktop-developer qnx-desktop-graphics qnx-desktop-extras"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop/files/." "$SRCROOT/"; }
do_build() { :; }
# The package must hold a file to exist (qnxpkg writes no package for an empty
# file list): the member list, share/qnx-desktop/qnx-desktop.list -- the header
# from files/ and one member per line, written from RUNTIME_DEPS so that the
# list on the target is what the package depends on.
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/qnx-desktop.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/qnx-desktop.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/qnx-desktop.list"
}
