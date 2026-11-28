# xfdesktop: the Xfce desktop manager -- draws the desktop background (image
# or colour, per monitor, optionally cycling through a folder), the icons on it
# (the files of ~/Desktop, home/file system/trash, minimized windows) and the
# right-click applications menu and the middle-click window list. On Wayland
# the desktop is a wlr-layer-shell surface in the background layer, so it needs
# a compositor with that protocol (labwc). xfdesktop-settings is its dialog.
VERSION=4.20.2
# Release tarball, as for every Xfce package here: it carries the generated
# sources a git checkout makes with xfce4-dev-tools.
TARBALL=https://archive.xfce.org/src/xfce/xfdesktop/4.20/xfdesktop-4.20.2.tar.bz2
SHA256=1d9bd76015fb6e9aca05e73cd998c7c66ed4fc8c10b626e08fc2eb7c39df3f7b
# garcon: the applications menu; thunar (libthunarx): the file manager's
# extensions in the icons' context menu; libnotify: mount/unmount messages;
# libyaml: the saved icon positions (~/.config/xfce4/desktop/icons.screen0.yaml).
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf exo garcon libxfce4windowing thunar libnotify libyaml gtk-layer-shell wayland"
# xfconf: every setting (channel xfce4-desktop) comes from xfconfd on the
# session bus; thunar: opening, renaming and deleting a file icon is done by the
# file manager over D-Bus (org.xfce.FileManager, with "Thunar" as the fallback).
# librsvg: the default background (xfce-x.svg) and most of the shipped ones
# are SVG, which gdk-pixbuf reads through librsvg's loader.
RUNTIME_DEPS="dbus xfconf thunar librsvg"
BUILDSYS=meson
# - Wayland only, as the rest of this Xfce (libxfce4windowing --disable-x11);
# - no test programs.
MESON_ARGS_EXTRA=( -Dx11=disabled -Dwayland=enabled -Ddesktop-menu=enabled -Ddesktop-icons=true
                   -Dfile-icons=true -Dthunarx=enabled -Dnotifications=enabled
                   -Dtests=false )
