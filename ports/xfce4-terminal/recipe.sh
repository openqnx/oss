# xfce4-terminal: terminal emulator (VTE). Version as in the QNX Everywhere
# 8.0.4r1 image; it builds against the Xfce 4.20 libraries in this tree.
VERSION=1.1.5
TARBALL=https://archive.xfce.org/src/apps/xfce4-terminal/1.1/xfce4-terminal-1.1.5.tar.xz
SHA256=3c5b1d3a01a9a113852ac0f77d1c85bf3a356b43de33ec805b21ceca7d6f0a63
# Also linked at run time (found by scripts/gen-dependency-index): gtk-layer-shell
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf vte pcre2 gtk-layer-shell"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dx11=disabled -Dwayland=enabled -Ddocs=false )
