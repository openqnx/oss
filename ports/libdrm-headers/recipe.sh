# libdrm-headers: libdrm 2.4.129's headers, header only -- the DRM kernel
# interface (drm.h, drm_fourcc.h, drm_mode.h, ...) and the library's own
# (xf86drm.h, xf86drmMode.h). The library is the QNX SDP's libdrm.so, which
# exports the newer functions (drmGetFormatName, drmModeCreateDumbBuffer,
# drmSyncobjEventfd, ...) but ships older headers that declare none of them,
# nor DRM_FORMAT_R16F and the other formats wlroots 0.20 uses. Installed to
# include/libdrm-uapi, ahead of the SDP's on the include path
# (scripts/lib/sdp-pc.sh's libdrm.pc).
VERSION=2.4.129; GIT=https://gitlab.freedesktop.org/mesa/drm.git; REF=libdrm-2.4.129
DEPS=""
BUILDSYS=custom
do_configure() { :; }
do_build() { :; }
do_install() {
  local d="$STAGE$PREFIX/include/libdrm-uapi"; mkdir -p "$d"
  install -m644 "$S"/include/drm/*.h "$S"/xf86drm.h "$S"/xf86drmMode.h "$d/"
}
