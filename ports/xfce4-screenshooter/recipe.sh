# xfce4-screenshooter: the Xfce screenshot tool -- the whole screen, the
# active window or a selected region, after a delay, with or without the
# pointer; save, copy to the clipboard or open with a program. Also a panel
# plugin. On Wayland it captures through wlr-screencopy (labwc has it, as the
# QNX screen share webrtc-screencast shows) and selects regions on a
# layer-shell surface.
VERSION=1.11.3
TARBALL=https://archive.xfce.org/src/apps/xfce4-screenshooter/1.11/xfce4-screenshooter-1.11.3.tar.xz
SHA256=1f6a14f7d1b0c440f31e24a8cc4fe2996185357fa786f0c2cdfe564ef673a710
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf xfce4-panel exo gtk-layer-shell wayland wayland-protocols"
RUNTIME_DEPS="libxfce4ui xfconf"
BUILDSYS=meson
# Wayland only, as the rest of this Xfce.
MESON_ARGS_EXTRA=( -Dx11=disabled -Dwayland=enabled -Dxfixes=disabled )
