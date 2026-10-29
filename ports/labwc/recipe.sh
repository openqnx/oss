# labwc: a stacking Wayland compositor (window manager) on wlroots -- the one
# QNX's Developer Desktop runs, with QNX's changes (patches/0001, from
# github.com/qnx-ports/labwc). Xfce's panel and applications run on it.
VERSION=0.20.2; GIT=https://github.com/labwc/labwc.git; REF=0.20.2
DEPS="wlroots wayland wayland-protocols libxkbcommon glib cairo pango libxml2 libsfdo libpng"
BUILDSYS=meson
# labwc will not start without a font (titles, menus) nor without keymaps.
RUNTIME_DEPS="font-dejavu xkeyboard-config"
MESON_ARGS_EXTRA=( -Dxwayland=disabled -Dsvg=disabled -Dnls=disabled -Dman-pages=disabled
                   -Dicon=enabled -Dstatic_analyzer=disabled )
# wlroots-0.20.pc requires libdrm (the SDP's, see scripts/lib/sdp-pc.sh).
source "$WS/scripts/lib/sdp-pc.sh"
do_configure() { qnx_sdp_pc && qnx_meson_setup; }
