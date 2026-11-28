# qnx-desktop-extras: reaching the desktop from elsewhere, and administration --
# Raspberry Pi OS' rpd-wayland-extras (wayvnc, grim, squeekboard) is about
# the same things. webrtc-screencast shows the desktop in a web browser (view
# only), gtk-vnc brings gvncviewer to look at other machines, OpenSSH the
# remote shell and file copy, sudo for a desktop user that is not root.
# This component holds no program of its own: it is a meta package. Installing
# it installs the components below (DEPS builds them first, RUNTIME_DEPS is
# what the package depends on; keep the two lines the same).
#
# To add when ported (not in ports/ yet -- move a name into DEPS and
# RUNTIME_DEPS once ports/<name> exists and its package is verified):
#   wayvnc                VNC server for the session (remote control, not only view)
#   grim / a screenshot front end
#   an on-screen keyboard (squeekboard), an input method (fcitx5 with Hangul)
VERSION=1.0
DEPS="webrtc-screencast gtk-vnc openssh sudo"
RUNTIME_DEPS="webrtc-screencast gtk-vnc openssh sudo"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop-extras/files/." "$SRCROOT/"; }
do_build() { :; }
# The package must hold a file to exist (qnxpkg writes no package for an empty
# file list): the member list, share/qnx-desktop/extras.list -- the header
# from files/ and one member per line, written from RUNTIME_DEPS so that the
# list on the target is what the package depends on.
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/extras.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/extras.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/extras.list"
}
