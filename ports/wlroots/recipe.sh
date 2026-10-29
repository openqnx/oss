# wlroots: the Wayland compositor library labwc is built on, with QNX's QNX
# Screen backend (patches/0001, from github.com/qnx-ports/wlroots): outputs
# are Screen windows, input is Screen events, rendering is GLES through the
# SDP's libEGL/libGLESv2. libdrm (drm_fourcc.h) and libgbm come from the SDP
# (8.0.5 ships both; newer DRM kernel headers: libdrm-headers). The Linux backends (DRM/KMS, libinput, X11) and
# Xwayland are off.
VERSION=0.20.2; GIT=https://gitlab.freedesktop.org/wlroots/wlroots.git; REF=0.20.2
DEPS="wayland wayland-protocols pixman libxkbcommon lcms2 libdrm-headers linux-headers-input-codes"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dbackends=qnx-screen -Drenderers=gles2 -Dallocators=auto
                   -Dxwayland=disabled -Dsession=disabled -Dexamples=false
                   -Dxcb-errors=disabled -Dwerror=false -Dcolor-management=enabled -Dlibliftoff=disabled )
source "$WS/scripts/lib/sdp-pc.sh"
do_configure() { qnx_sdp_pc && qnx_meson_setup; }
