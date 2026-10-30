# xwayland: the X server as a Wayland client (X.Org's xserver, Xwayland only).
# On the QNX desktop it is an X display in a window ("Xwayland :1
# -geometry WxH", rootful): X11 programs and vglclient -- VirtualGL's client,
# which draws OpenGL frames rendered on a Linux machine (RViz in an Ubuntu
# guest) -- show there. Software rendering: no glamor/DRI3 (no desktop GL or
# DRM render nodes on QNX); no MIT-SHM (no System V shared memory on QNX).
VERSION=24.1.14; GIT=https://gitlab.freedesktop.org/xorg/xserver.git; REF=xwayland-24.1.14
DEPS="wayland wayland-protocols pixman xorgproto xtrans libxau libxdmcp libxfont2 libxkbfile libxcvt libxshmfence libdrm-headers linux-headers-input-codes libmd xkbcomp xkeyboard-config font-util"
RUNTIME_DEPS="xkbcomp xkeyboard-config"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dglamor=false -Dglx=false -Ddri3=false -Ddrm=false -Dmitshm=false -Dxvfb=false
                   -Dxdmcp=false -Dxdm-auth-1=false -Dsecure-rpc=false -Dlibdecor=false -Dxwayland_ei=false
                   -Dxselinux=false -Dsystemd_notify=false -Ddocs=false -Ddevel-docs=false -Ddocs-pdf=false
                   -Dsha1=libmd -Dlisten_tcp=true -Dxkb_dir=/usr/local/share/X11/xkb -Dxkb_bin_dir=/usr/local/bin
                   -Dxkb_output_dir=/tmp )
# Xwayland requires libdrm's headers even without DRI3: the SDP's libdrm
# through scripts/lib/sdp-pc.sh, as for wlroots.
source "$WS/scripts/lib/sdp-pc.sh"
do_configure() { qnx_sdp_pc && qnx_meson_setup; }
