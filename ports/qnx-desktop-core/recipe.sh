# qnx-desktop-core: the session of the desktop -- what Raspberry Pi OS puts in
# rpd-wayland-core. oss-desktop (the start script and the labwc/Xfce defaults)
# with every program the session runs: labwc and swaybg, the Xfce session,
# settings daemon, panel, application finder, Thunar and the terminal; the
# libraries' own services they talk to (xfconf, garcon's menus, exo's helpers,
# tumbler's thumbnails, D-Bus, notifications through libnotify); Xwayland for
# X11 programs; and the data a session needs at run time (keyboard layouts,
# MIME database, desktop-file tools, GSettings schemas, TLS for GIO, the
# secret store); the panel's network indicator (oss-network-plugin) and
# volume control (xfce4-alsa-plugin, on the QNX image's libasound -- "No
# sound card found" when io-snd is not running); the log-in screen
# (oss-greeter, shown when qnx-desktop.conf has DESKTOP_GREETER=yes).
# It also installs the boot entry of the session, bin/qnx-desktop-start, for
# a board image whose startup script runs /data/oss/etc/rc.local once Screen
# is up (files/; share/qnx-desktop/rc.local.sample is that rc.local).
# Its settings are in /data/oss/etc/qnx-desktop.conf (see the script's
# header), among them TZ, the time zone of the session: QNX's C library reads
# only POSIX strings (KST-9), so an area name (TZ=Asia/Seoul) is turned into
# one from tzdata; without a setting the system's (setconf _CS_TIMEZONE) is
# used, else UTC. The system D-Bus needs a "messagebus" account that the
# package does not add (/etc/passwd belongs to the BSP): qnx-desktop-start
# --check prints the two lines to add; the session runs without it.
# This component holds no program of its own: it is a meta package. Installing
# it installs the components below (DEPS builds them first, RUNTIME_DEPS is
# what the package depends on; keep the two lines the same).
#
# To add when ported (not in ports/ yet -- move a name into DEPS and
# RUNTIME_DEPS once ports/<name> exists and its package is verified):
#   kanshi wlopm          output profiles and output power (rpd-wayland-core has them)
#   a greeter / log-in screen, a first-boot wizard (not ported, not written yet)
VERSION=1.0
DEPS="oss-desktop oss-greeter labwc swaybg xfce4-panel oss-network-plugin xfce4-alsa-plugin xfce4-session xfce4-settings xfce4-terminal thunar xfce4-appfinder xfconf garcon exo tumbler dbus libnotify xwayland xkeyboard-config xdg-utils shared-mime-info desktop-file-utils gsettings-desktop-schemas glib-networking gnome-keyring dconf xfce4-notifyd xfdesktop swayidle swaylock tzdata librsvg"
RUNTIME_DEPS="oss-desktop oss-greeter labwc swaybg xfce4-panel oss-network-plugin xfce4-alsa-plugin xfce4-session xfce4-settings xfce4-terminal thunar xfce4-appfinder xfconf garcon exo tumbler dbus libnotify xwayland xkeyboard-config xdg-utils shared-mime-info desktop-file-utils gsettings-desktop-schemas glib-networking gnome-keyring dconf xfce4-notifyd xfdesktop swayidle swaylock tzdata librsvg"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop-core/files/." "$SRCROOT/"; }
do_build() { :; }
# The package must hold a file to exist (qnxpkg writes no package for an empty
# file list): the member list, share/qnx-desktop/core.list -- the header
# from files/ and one member per line, written from RUNTIME_DEPS so that the
# list on the target is what the package depends on.
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/core.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/core.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/core.list"
  install -Dm755 "$S/bin/qnx-desktop-start" "$STAGE$PREFIX/bin/qnx-desktop-start"
  install -Dm644 "$S/share/qnx-desktop/rc.local.sample" "$STAGE$PREFIX/share/qnx-desktop/rc.local.sample"
}
