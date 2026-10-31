# pkg-config files for QNX SDP libraries that come without one: libdrm, gbm,
# EGL, GLESv2 (SDP 8.0.5 ships the libraries and headers, no .pc). libdrm's
# kernel headers come from the libdrm-headers component (put it in DEPS). wlroots
# and what builds on it look them up by pkg-config. Opt-in per recipe, so
# nothing else starts finding (and enabling) them:
#   source "$WS/scripts/lib/sdp-pc.sh"
#   do_configure() { qnx_sdp_pc && qnx_meson_setup; }
#
# PKG_CONFIG_SYSROOT_DIR is the staging tree, and pkg-config puts it in front
# of every -I/-L: so the .pc files name no directories (qcc already searches
# the SDP's usr/include and usr/lib), except libdrm's header directory, which
# is reached through a link inside the staging tree's .toolchain/.
qnx_sdp_pc() {
  local d="$STAGE/.toolchain/sdp-pkgconfig" inc="$STAGE/.toolchain/sdp-include"
  mkdir -p "$d" "$inc"
  # The SDP's own directory, not the workspace's linkroot: a link into the
  # workspace breaks when the workspace moves (it did).
  ln -sfn "${QNX_TARGET_SDP:-$QNX_TARGET}/usr/include/libdrm" "$inc/libdrm"
  # The SDP's libdrm.so with newer kernel interface headers in front of its
  # own (the libdrm-headers component, libdrm 2.4.129's; xf86drm.h stays the
  # SDP's). The SDP has two: libdrm.so.1, the old ABI that plain -ldrm
  # finds, and libdrm.so.2, the current one with the functions these headers
  # declare -- linked by name. The version is the headers'.
  local v; v=$(sed -n 's/^VERSION=\([^;]*\).*/\1/p' "$WS/ports/libdrm-headers/recipe.sh")
  printf 'Name: libdrm\nDescription: QNX SDP libdrm, libdrm %s kernel headers\nVersion: %s\nCflags: -I%s/include/libdrm-uapi -I/.toolchain/sdp-include/libdrm\nLibs: -l:libdrm.so.2\n' "$v" "$v" "$PREFIX" > "$d/libdrm.pc"
  printf 'Name: gbm\nDescription: QNX SDP libgbm\nVersion: 24.0.0\nLibs: -lgbm\nRequires.private: libdrm\n' > "$d/gbm.pc"
  printf 'Name: egl\nDescription: QNX SDP EGL\nVersion: 1.5\nLibs: -lEGL\n' > "$d/egl.pc"
  printf 'Name: glesv2\nDescription: QNX SDP OpenGL ES 2/3\nVersion: 3.2\nLibs: -lGLESv2\n' > "$d/glesv2.pc"
  export PKG_CONFIG_LIBDIR="$d:$PKG_CONFIG_LIBDIR"
  QNX_SDP_PC_DIR=$d
}
# The driver's meson configure step, for recipes that override do_configure.
# Meson takes pkg-config's search path from the cross file, not from the
# environment: a copy of it with the SDP .pc directory in front.
qnx_meson_setup() {
  local nf=() cf="$QNX_MESON_DIR/meson-$arch.ini"
  [ -f "$QNX_MESON_DIR/meson-native.ini" ] && nf=( --native-file "$QNX_MESON_DIR/meson-native.ini" )
  if [ -n "$QNX_SDP_PC_DIR" ]; then
    sed "s#^pkg_config_libdir *= *\[#pkg_config_libdir = ['$QNX_SDP_PC_DIR', #" "$cf" > "$B/.cross-sdp.ini"
    cf=$B/.cross-sdp.ini
  fi
  meson setup "$B" "$S" --cross-file "$cf" "${nf[@]}" \
    --prefix="$PREFIX" --buildtype=release --default-library=both "${MESON_ARGS_EXTRA[@]}"
}
