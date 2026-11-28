# flutter-counter-example: Flutter's own "counter" starter app (flutter create)
# built for QNX -- the release AOT snapshot (libapp.so) and assets, run by
# qnx-flutter-wayland: `flutter-counter` opens it on the desktop.
#
# How a Flutter app is built for QNX (the same for any app):
#  1. flutter build bundle --release   -> flutter_assets/ (fonts, images, ...)
#  2. the frontend server of the Flutter checkout compiles the Dart code to a
#     kernel file (AOT, tree shaken) against the engine's flutter_patched_sdk;
#  3. gen_snapshot of the QNX engine build (a build-host program targeting the
#     QNX CPU) turns it into libapp.so (app-aot-elf).
# Both the Flutter tool and the engine build come from flutter-engine's tree.
VERSION=1.0
DEPS="flutter-engine qnx-flutter-wayland"
RUNTIME_DEPS="qnx-flutter-wayland"
BUILDSYS=custom
_cpu() { [ "$QNX_ARCH" = aarch64le ] && echo arm64 || echo x64; }
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/flutter-counter-example/files/." "$SRCROOT/"; }
do_build() {
  local F="$SRC_DIR/flutter-engine" A="$B/counter"
  local E="$F/engine/src/out/qnx_release_$(_cpu)"
  [ -x "$E/clang_x64/gen_snapshot" ] || { echo "gen_snapshot missing: build flutter-engine first"; return 1; }
  export PATH="$F/bin:$PATH" CI=true FLUTTER_SUPPRESS_ANALYTICS=true PUB_CACHE="$B/pub-cache"
  flutter config --no-analytics >/dev/null 2>&1
  rm -rf "$A" && flutter create --platforms=linux --project-name counter "$A" >/dev/null || return 1
  cd "$A" && flutter pub get && \
  flutter build bundle --release --target-platform=linux-x64 --asset-dir="$B/bundle/flutter_assets" && \
  "$F/bin/cache/dart-sdk/bin/dartaotruntime" "$F/bin/cache/artifacts/engine/linux-x64/frontend_server_aot.dart.snapshot" \
    --sdk-root "$E/flutter_patched_sdk/" --target=flutter --aot --tfa -Ddart.vm.product=true -Ddart.vm.profile=false \
    --packages .dart_tool/package_config.json --output-dill "$B/app.dill" package:counter/main.dart && \
  "$E/clang_x64/gen_snapshot" --deterministic --snapshot_kind=app-aot-elf --elf="$B/bundle/libapp.so" --strip "$B/app.dill"
}
do_install() {
  local D="$STAGE$PREFIX/share/flutter-apps/counter"
  mkdir -p "$D" && cp -a "$B/bundle/." "$D/" && \
  install -Dm755 "$S/flutter-counter" "$STAGE$PREFIX/bin/flutter-counter" && \
  sed -i "s|@PREFIX@|$PREFIX|g" "$STAGE$PREFIX/bin/flutter-counter" && \
  install -Dm644 "$S/flutter-counter.desktop" "$STAGE$PREFIX/share/applications/flutter-counter.desktop"
}
