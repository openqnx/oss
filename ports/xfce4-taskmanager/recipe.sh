# xfce4-taskmanager: the Xfce task manager -- the process list (tree, CPU,
# memory, priority; end, kill, stop, continue and renice from the menu) with
# CPU and memory graphs. Raspberry Pi OS has lxtask in this place.
VERSION=1.6.0
TARBALL=https://archive.xfce.org/src/apps/xfce4-taskmanager/1.6/xfce4-taskmanager-1.6.0.tar.xz
SHA256=29bdc7840ab8b9025f6c0e456a83a31090d1c9fd9e26b359baa4a4010cfb0b90
# libxmu: meson.build requires it, the program does not link it (X11 code is off).
DEPS="glib gtk3 cairo libxmu libxfce4util libxfce4ui xfconf"
RUNTIME_DEPS="libxfce4ui xfconf"
BUILDSYS=meson
# patches/0001: the QNX backend (processes from /proc and devctl()).
# Wayland only, as the rest of this Xfce: no X11, no libwnck (window icons and
# names of the processes' windows, X11 only).
MESON_ARGS_EXTRA=( -Dx11=disabled -Dwnck=disabled )
