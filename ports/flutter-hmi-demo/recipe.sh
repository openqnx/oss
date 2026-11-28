# flutter-hmi-demo: a vehicle HMI mock-up written for this tree (files/, Dart):
# instrument cluster (speed and rpm gauges, gear, turn signals, tell-tales,
# fuel/coolant/battery), navigation map with a moving route, media player,
# climate control, vehicle status (tyre pressures, drive modes), phone and
# settings on one 1280x720 screen, driven by a simulated drive cycle. Pure
# Flutter (CustomPainter and Material widgets, no packages), pointer input,
# software rendering: `flutter-hmi-demo` opens it on the desktop through
# qnx-flutter-wayland (--size 1280x720; HMI_SIZE=WxH overrides).
#
# Built like flutter-counter-example (see there): flutter build bundle for the
# assets, the Flutter checkout's frontend server for the AOT kernel, and the
# QNX engine build's gen_snapshot for libapp.so.
VERSION=1.0
DEPS="flutter-engine qnx-flutter-wayland"
RUNTIME_DEPS="qnx-flutter-wayland"
BUILDSYS=custom
_cpu() { [ "$QNX_ARCH" = aarch64le ] && echo arm64 || echo x64; }
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/flutter-hmi-demo/files/." "$SRCROOT/"; }
do_build() {
  local F="$SRC_DIR/flutter-engine" A="$B/app"
  local E="$F/engine/src/out/qnx_release_$(_cpu)"
  [ -x "$E/clang_x64/gen_snapshot" ] || { echo "gen_snapshot missing: build flutter-engine first"; return 1; }
  export PATH="$F/bin:$PATH" CI=true FLUTTER_SUPPRESS_ANALYTICS=true PUB_CACHE="$B/pub-cache"
  flutter config --no-analytics >/dev/null 2>&1
  rm -rf "$A" && mkdir -p "$A" && cp -a "$S/pubspec.yaml" "$S/lib" "$A/" || return 1
  cd "$A" && flutter pub get && \
  flutter build bundle --release --target-platform=linux-x64 --asset-dir="$B/bundle/flutter_assets" && \
  "$F/bin/cache/dart-sdk/bin/dartaotruntime" "$F/bin/cache/artifacts/engine/linux-x64/frontend_server_aot.dart.snapshot" \
    --sdk-root "$E/flutter_patched_sdk/" --target=flutter --aot --tfa -Ddart.vm.product=true -Ddart.vm.profile=false \
    --packages .dart_tool/package_config.json --output-dill "$B/app.dill" package:hmi_demo/main.dart && \
  "$E/clang_x64/gen_snapshot" --deterministic --snapshot_kind=app-aot-elf --elf="$B/bundle/libapp.so" --strip "$B/app.dill"
}
do_install() {
  local D="$STAGE$PREFIX/share/flutter-apps/hmi-demo"
  mkdir -p "$D" && cp -a "$B/bundle/." "$D/" && \
  install -Dm755 "$S/flutter-hmi-demo" "$STAGE$PREFIX/bin/flutter-hmi-demo" && \
  sed -i "s|@PREFIX@|$PREFIX|g" "$STAGE$PREFIX/bin/flutter-hmi-demo" && \
  install -Dm644 "$S/flutter-hmi-demo.desktop" "$STAGE$PREFIX/share/applications/flutter-hmi-demo.desktop"
}
