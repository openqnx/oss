# xfce4-notifyd: the Xfce notification daemon -- the org.freedesktop.Notifications
# service that shows the pop-ups programs send with libnotify/notify-send, keeps
# a log of them (SQLite, ~/.cache/xfce4/notifyd/log.sqlite), a settings dialog
# (xfce4-notifyd-config) and a panel plugin with the recent notifications and a
# "do not disturb" switch. The session bus starts it on the first notification
# (share/dbus-1/services), and etc/xdg/autostart starts it with the session.
VERSION=0.9.7
# Release tarball, as for every Xfce package here: it carries the generated
# sources a git checkout makes with xfce4-dev-tools.
TARBALL=https://archive.xfce.org/src/apps/xfce4-notifyd/0.9/xfce4-notifyd-0.9.7.tar.bz2
SHA256=170d18fd5f40cce823ffc7ae3d7e21644007c3f45808ab4835f0401d21b3516a
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf xfce4-panel libnotify sqlite gtk-layer-shell wayland"
# dbus: the daemon is a session-bus service; xfconf: its settings (xfconfd).
RUNTIME_DEPS="dbus xfconf"
BUILDSYS=meson
# - Wayland only, as the rest of this Xfce (libxfce4windowing --disable-x11):
#   the pop-ups are wlr-layer-shell surfaces (gtk-layer-shell) on labwc;
# - no notification sounds: they need libcanberra, which is not ported;
# - no systemd user unit (QNX has no systemd): the D-Bus activation files and
#   the XDG autostart entry start the daemon.
MESON_ARGS_EXTRA=( -Dx11=disabled -Dwayland=enabled -Dsound=disabled -Dsystemd=disabled
                   -Ddbus-start-daemon=true -Dxdg-autostart=true -Dtests=false )
