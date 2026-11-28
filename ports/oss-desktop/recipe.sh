# oss-desktop: a desktop session on QNX Screen from the ported components, in
# the way of QNX Everywhere's Developer Desktop -- labwc (window manager, on
# wlroots' QNX Screen backend), swaybg (background colour) and the Xfce
# session (xfce4-session: settings daemon, the panel, Thunar, and xfdesktop --
# the wallpaper and desktop icons -- when installed): one panel at the top
# with the Applications menu, launchers (terminal, files, web browser), the
# clock and the log out / restart / shut down buttons. The root menu (right
# click on the background) lists every installed application and runs the
# port test suite in a terminal.
# This component is only the session: a start script, oss-desktop, labwc's
# configuration and the Xfce defaults (files/); the programs are the
# components below. An image that boots into it: scripts/make-ported-image --desktop.
VERSION=1.1
DEPS="labwc swaybg xfce4-panel xfce4-session xfce4-settings xfce4-terminal thunar xfce4-appfinder dbus adwaita-icon-theme hicolor-icon-theme lxde-icon-theme font-dejavu"
RUNTIME_DEPS="labwc swaybg xfce4-panel xfce4-session xfce4-settings xfce4-terminal thunar xfce4-appfinder dbus adwaita-icon-theme hicolor-icon-theme lxde-icon-theme font-dejavu"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/oss-desktop/files/." "$SRCROOT/"; }
do_build() { :; }
do_install() {
  install -Dm755 "$S/bin/oss-desktop" "$STAGE$PREFIX/bin/oss-desktop"
  install -Dm755 "$S/bin/oss-desktop-menu" "$STAGE$PREFIX/bin/oss-desktop-menu"
  install -d "$STAGE$PREFIX/share/oss-desktop" && cp -a "$S/share/oss-desktop/." "$STAGE$PREFIX/share/oss-desktop/" && \
  install -Dm644 "$S/share/glib-2.0/schemas/50_oss-desktop.gschema.override" \
                 "$STAGE$PREFIX/share/glib-2.0/schemas/50_oss-desktop.gschema.override"
}
