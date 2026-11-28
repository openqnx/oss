# qnx-flutter-wayland: a small Flutter embedder for QNX on Wayland (labwc).
# Written for this tree (files/qnx_flutter_wayland.c, C): an xdg-shell window
# on wl_shm buffers that the engine's software renderer draws into; pointer
# (mouse) input; its own platform task runner (poll on the Wayland fd, a wake
# pipe and the next task's time). Keyboard and text input are not done yet.
#
#   qnx-flutter-wayland <bundle dir>   (flutter_assets/, libapp.so[, icudtl.dat])
#
# QNX's loader does not expand $ORIGIN in RPATH/RUNPATH, so the program names
# $PREFIX/lib as its RPATH (DT_RPATH) to find libflutter_engine.so and
# libwayland-client. The loader uses it for the program's own NEEDED only, not
# for those libraries' (libffi of libwayland-client): outside a desktop session
# (whose LD_LIBRARY_PATH has $PREFIX/lib) an app's launcher sets
# LD_LIBRARY_PATH, as flutter-counter does.
VERSION=0.1
DEPS="flutter-engine wayland wayland-protocols"
RUNTIME_DEPS="flutter-engine wayland"
BUILDSYS=custom
_wls="$WS/hosttools/bin/wayland-scanner"; [ -x "$_wls" ] || _wls=$(command -v wayland-scanner)
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-flutter-wayland/files/." "$SRCROOT/"; }
do_build() {
  local X="$STAGE$PREFIX/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml"
  mkdir -p "$B" && cd "$B" && \
  "$_wls" client-header "$X" xdg-shell-client-protocol.h && \
  "$_wls" private-code "$X" xdg-shell-protocol.c && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -I. -I"$STAGE$PREFIX/include" \
      -DICU_DATA_DEFAULT="\"$PREFIX/share/flutter/icudtl.dat\"" \
      -o qnx-flutter-wayland "$S/qnx_flutter_wayland.c" xdg-shell-protocol.c \
      -L"$STAGE$PREFIX/lib" -Wl,-rpath-link,"$STAGE$PREFIX/lib" -Wl,--disable-new-dtags -Wl,-rpath,"$PREFIX/lib" \
      -lwayland-client -lflutter_engine
}
do_install() { install -Dm755 "$B/qnx-flutter-wayland" "$STAGE$PREFIX/bin/qnx-flutter-wayland"; }
