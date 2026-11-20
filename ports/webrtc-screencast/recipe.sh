# webrtc-screencast: the desktop of the target in a web browser. A Wayland
# client that asks the compositor (labwc, any wlroots one) for the picture of
# an output (wlr-screencopy), sends it as a WebRTC video track (libwebrtc of
# the webrtc component, VP8 in software) and serves the page a browser
# opens, with the SDP exchange, from its own small HTTP server. View only, or
# with --input the browser's mouse and keyboard, sent over a data channel and
# given to the compositor's virtual pointer and keyboard (a US layout).
# Nothing of QNX Screen is used directly: the picture is the compositor's.
#
# This component is the program (files/): one static binary on libwebrtc,
# the session starter and its XDG autostart entry, so that a desktop image
# (oss-desktop) shows itself at http://<address>:8080/ once it is installed.
VERSION=1.1
DEPS="webrtc wayland wlr-protocols libxkbcommon"
RUNTIME_DEPS="wayland libxkbcommon xkeyboard-config"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/webrtc-screencast/files/." "$SRCROOT/"; }
# The compile flags are those of the webrtc build itself (its
# obj/examples/peerconnection_client.ninja): libwebrtc.a is built without
# RTTI and exceptions, and its headers change layout with these macros.
_WDEFS="-D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -D_FILE_OFFSET_BITS=64 \
-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -DQNX -D_QNX_SOURCE -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_NONE \
-DNDEBUG -DWEBRTC_STRICT_FIELD_TRIALS=0 -DWEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE -DRTC_ENABLE_VP9 \
-DRTC_DAV1D_IN_INTERNAL_DECODER_FACTORY -DWEBRTC_HAVE_SCTP -DWEBRTC_NON_STATIC_TRACE_EVENT_HANDLERS=1 \
-DWEBRTC_POSIX -DWEBRTC_QNX -DABSL_ALLOCATOR_NOTHROW=1 -DLIBYUV_DISABLE_NEON -DLIBYUV_DISABLE_SVE \
-DLIBYUV_DISABLE_SME -DHAVE_WEBRTC_VIDEO"
do_build() {
  local W="$STAGE$PREFIX/include/webrtc" x="$STAGE$PREFIX/share/wlr-protocols/unstable/wlr-screencopy-unstable-v1.xml"
  local wls="$WS/hosttools/bin/wayland-scanner"; [ -x "$wls" ] || wls=$(command -v wayland-scanner)
  [ -f "$STAGE$PREFIX/lib/webrtc/libwebrtc.a" ] || { echo "the webrtc component has no libwebrtc.a (rebuild webrtc)"; return 1; }
  mkdir -p "$B" && cd "$B" && \
  local n f; for n in wlr-screencopy-unstable-v1 wlr-virtual-pointer-unstable-v1 virtual-keyboard-unstable-v1; do
    f="$(dirname "$x")/$n.xml"; [ -f "$f" ] || f="$S/$n.xml"    # virtual-keyboard: wlroots' copy, in files/
    "$wls" client-header "$f" "$n-client-protocol.h" && "$wls" private-code "$f" "$n-protocol.c" && \
    qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -fPIC -I. -I"$STAGE$PREFIX/include" -c "$n-protocol.c" -o "$n-protocol.o" || return 1
  done
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -fPIC -I. -I"$S" -I"$STAGE$PREFIX/include" \
      -c "$S/input.c" -o input.o && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -fPIC -I. -I"$S" -I"$STAGE$PREFIX/include" \
      -c "$S/capture.c" -o capture.o && \
  q++ -V"${QCC_TARGET}_cxx" $_WDEFS -std=c++17 -fno-exceptions -fno-rtti -O2 -fPIC -fdata-sections -ffunction-sections \
      -Wall -Wno-deprecated-declarations -Wno-psabi \
      -I"$W" -I"$W/third_party/abseil-cpp" -I"$W/third_party/libyuv/include" -I"$QNX_TARGET/usr/include/io-sock" -I"$S" \
      -c "$S/main.cc" -o main.o && \
  # -lscreen -lEGL -lGLESv2: libwebrtc.a's own QNX camera and renderer code
  # (not used here, dropped by --gc-sections where nothing refers to it).
  q++ -V"${QCC_TARGET}_cxx" -Wl,--gc-sections -Wl,--as-needed -o webrtc-screencast main.o capture.o input.o *-protocol.o \
      "$STAGE$PREFIX/lib/webrtc/libwebrtc.a" -L"$STAGE$PREFIX/lib" -lwayland-client -lxkbcommon \
      -lsocket -lm
}
do_install() {
  install -Dm755 "$B/webrtc-screencast" "$STAGE$PREFIX/bin/webrtc-screencast" && \
  install -Dm755 "$S/webrtc-screencast-session" "$STAGE$PREFIX/bin/webrtc-screencast-session" && \
  install -Dm644 "$S/webrtc-screencast.desktop" "$STAGE$PREFIX/etc/xdg/autostart/webrtc-screencast.desktop" && \
  install -Dm644 "$S/etc/webrtc-screencast.conf" "$STAGE$PREFIX/etc/webrtc-screencast.conf"
}
