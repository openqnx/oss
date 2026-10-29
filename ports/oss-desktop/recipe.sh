# oss-desktop: a desktop session on QNX Screen from the ported components, in
# the spirit of QNX Everywhere's Developer Desktop -- labwc (window manager,
# on wlroots' QNX Screen backend), swaybg (background), the Xfce panel with
# an application menu, the Xfce terminal and file manager. The root menu
# (right click on the background) lists every installed application
# (.desktop entry) and runs the port test suite in a terminal.
# This component is only the session: a start script, oss-desktop, and
# labwc's configuration (files/); the programs are the components below.
VERSION=1.0
DEPS="labwc swaybg xfce4-panel xfce4-terminal thunar xfce4-appfinder dbus hicolor-icon-theme lxde-icon-theme font-dejavu"
RUNTIME_DEPS="labwc swaybg xfce4-panel xfce4-terminal thunar xfce4-appfinder dbus hicolor-icon-theme lxde-icon-theme font-dejavu"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/oss-desktop/files/." "$SRCROOT/"; }
do_build() { :; }
do_install() {
  install -Dm755 "$S/bin/oss-desktop" "$STAGE$PREFIX/bin/oss-desktop"
  install -Dm755 "$S/bin/oss-desktop-menu" "$STAGE$PREFIX/bin/oss-desktop-menu"
  install -d "$STAGE$PREFIX/share/oss-desktop" && cp -a "$S/share/oss-desktop/." "$STAGE$PREFIX/share/oss-desktop/"
}
