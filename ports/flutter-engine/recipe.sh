VERSION=3.50.1; GIT=https://github.com/flutter/flutter.git; REF=3.50.1
# The Flutter engine for QNX: libflutter_engine.so (the embedder API,
# shell/platform/embedder/embedder.h), release mode, software rendering on the
# CPU (Skia raster; no GL/Vulkan yet). An embedder -- qnx-flutter-wayland, a
# Wayland window on labwc -- loads it together with an app's AOT snapshot.
#
# Upstream builds the engine only for its own platforms; it knows QNX just for
# the standalone Impeller library (qcc, arm64). patches/ add the embedder
# engine and the Dart VM for QNX (each patch says why): a QNX clang toolchain
# in the engine's GN build, Dart's QNX layer (a poll(2) event handler), the
# engine's libc++ with a "C"-locale base for QNX, fml's message loop, fonts
# from /usr/local/share/fonts.
#
# The source is Flutter's monorepo at the release tag plus its dependencies
# (gclient, depot_tools: about 28 GB). It is compiled with a build-host clang
# that knows QNX -- LLVM with the same qnx-ports patch as ports/llvm, built
# once into hosttools/qnx-clang -- against a sysroot made of the SDP's target
# headers and libraries. Snapshots of Dart code use the Linux target ABI, the
# same as QNX's on x86_64 and aarch64, so app AOT code (libapp.so) comes from
# this build's gen_snapshot (flutter-counter-example shows how).
BUILDSYS=custom
FLUTTER_LLVM=22.1.8

_cpu() { [ "$QNX_ARCH" = aarch64le ] && echo arm64 || echo x64; }
_depot() { echo "$WS/hosttools/depot_tools"; }

do_fetch() {
  local D; D=$(_depot)
  [ -d "$D/.git" ] || git clone -q https://chromium.googlesource.com/chromium/tools/depot_tools.git "$D" || return 1
  export PATH="$D:$PATH" DEPOT_TOOLS_UPDATE=0 DEPOT_TOOLS_METRICS=0
  git clone -q --branch "$REF" --depth 1 "$GIT" "$SRCROOT" || return 1
  cd "$SRCROOT" && cp engine/scripts/standard.gclient .gclient || return 1
  sed -i 's/"download_android_deps"[^,]*,/"download_android_deps": False,/' .gclient
  gclient sync -D --no-history --shallow -j 8
}

# A build-host clang/lld with QNX support (qnx-ports), once for all components.
_qnx_clang() {
  local H="$WS/hosttools/qnx-clang" T="$WS/dist/llvm-project-$FLUTTER_LLVM.src.tar.xz" W="$BUILD_DIR/qnx-clang"
  [ -x "$H/bin/clang" ] && return 0
  [ -s "$T" ] || curl -fsSL --retry 3 -o "$T" \
    "https://github.com/llvm/llvm-project/releases/download/llvmorg-$FLUTTER_LLVM/llvm-project-$FLUTTER_LLVM.src.tar.xz" || return 1
  rm -rf "$W" && mkdir -p "$W" && tar -xJf "$T" -C "$W" --strip-components=1 || return 1
  local p; for p in "$RECIPE_DIR"/llvm/patches/*.patch; do patch -p1 -d "$W" < "$p" || return 1; done
  env -u CC -u CXX cmake -S "$W/llvm" -B "$W/build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$H" \
    -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_TARGETS_TO_BUILD="X86;AArch64" -DLLVM_INCLUDE_TESTS=OFF \
    -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_BUILD_DOCS=OFF -DLLVM_APPEND_VC_REV=OFF \
    -DLLVM_ENABLE_ZLIB=ON -DLLVM_ENABLE_TERMINFO=OFF -DLLVM_ENABLE_LIBEDIT=OFF -DCLANG_DEFAULT_LINKER=lld || return 1
  ninja -C "$W/build" install-clang install-clang-resource-headers install-lld install-llvm-ar install-llvm-nm \
    install-llvm-objcopy install-llvm-strip install-llvm-readelf install-llvm-ranlib install-llvm-objdump || return 1
  rm -rf "$W"
}

# The SDP's target headers and libraries in the layout clang's QNX driver
# expects (<sysroot>/usr/include, <sysroot>/usr/lib), plus the gcc runtime.
_sysroot() {
  local S="$B/sysroot" g gd d f
  g=$([ "$QNX_ARCH" = aarch64le ] && echo aarch64-unknown-nto-qnx8.0.0 || echo x86_64-pc-nto-qnx8.0.0)
  gd=$(ls -d "$QNX_HOST"/usr/lib/gcc/$g/* | head -n 1)
  rm -rf "$S" && mkdir -p "$S/usr/lib" && ln -s "$QNX_TARGET/usr/include" "$S/usr/include" || return 1
  for d in "$QNX_TARGET/$QNX_ARCH/lib" "$QNX_TARGET/$QNX_ARCH/usr/lib" "$gd"; do
    for f in "$d"/*; do ln -sf "$f" "$S/usr/lib/"; done
  done
}

do_configure() {
  _qnx_clang || return 1
  _sysroot || return 1
  # The engine's GN files run helper scripts with vpython3 (depot_tools).
  mkdir -p "$B/bin" && printf '#!/bin/sh\nexec python3 "$@"\n' > "$B/bin/vpython3" && chmod +x "$B/bin/vpython3"
  export PATH="$B/bin:$(_depot):$PATH" DEPOT_TOOLS_UPDATE=0
  local c; c=$(_cpu)
  cd "$S/engine/src" && python3 ./flutter/tools/gn --target-os qnx --linux-cpu "$c" --runtime-mode release --no-lto \
    --no-enable-unittests --no-goma --no-rbe --embedder-for-target --gn-args qnx_use_clang=true \
    --gn-args "qnx_clang_dir=\"$WS/hosttools/qnx-clang\"" \
    --gn-args "qnx_sysroot_$c=\"$B/sysroot\"" \
    --gn-args shell_enable_vulkan=false --gn-args shell_enable_gl=false --gn-args skia_use_vulkan=false \
    --gn-args skia_enable_ganesh=true --gn-args skia_use_freetype=true \
    --gn-args skia_enable_fontmgr_custom_directory=true --gn-args skia_enable_fontmgr_custom_empty=true
}

do_build() {
  export PATH="$B/bin:$(_depot):$PATH"
  ninja -C "$S/engine/src/out/qnx_release_$(_cpu)" flutter/shell/platform/embedder:flutter_engine \
    flutter/lib/snapshot:generate_snapshot_bins
}

do_install() {
  local O="$S/engine/src/out/qnx_release_$(_cpu)"
  install -D -m 755 "$O/libflutter_engine.so" "$STAGE$PREFIX/lib/libflutter_engine.so"
  install -D -m 644 "$S/engine/src/flutter/shell/platform/embedder/embedder.h" "$STAGE$PREFIX/include/flutter_embedder.h"
  install -D -m 644 "$O/icudtl.dat" "$STAGE$PREFIX/share/flutter/icudtl.dat"
}
