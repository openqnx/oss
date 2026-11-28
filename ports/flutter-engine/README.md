# flutter-engine 3.50.1

> The Flutter engine (embedder API, software rendering) for QNX

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/flutter/flutter.git (tag `3.50.1`) |
| Build system | custom (see recipe) |
| QNX patches | 8 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_flutter_engine`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `flutter-engine` and `flutter-engine-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild flutter-engine` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.50.1 --recurse-submodules --shallow-submodules \
    https://github.com/flutter/flutter.git src/flutter-engine
```

### 3. Apply the QNX patches

8 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/flutter-engine < ports/flutter-engine/patches/0001-qnx-engine-build-toolchain-fml-txt.patch
patch -p1 -d src/flutter-engine < ports/flutter-engine/patches/0002-qnx-dart-vm.patch
patch -p1 -d src/flutter-engine < ports/flutter-engine/patches/0003-qnx-libcxx.patch
patch -p1 -d src/flutter-engine < ports/flutter-engine/patches/0004-qnx-libcxxabi.patch
patch -p1 -d src/flutter-engine < ports/flutter-engine/patches/0005-qnx-boringssl-urandom.patch
patch -p1 -d src/flutter-engine < ports/flutter-engine/patches/0006-qnx-skia-no-locale-t.patch
patch -p1 -d src/flutter-engine < ports/flutter-engine/patches/0007-qnx-zlib-no-arm-cpu-features.patch
patch -p1 -d src/flutter-engine < ports/flutter-engine/patches/0008-qnx-swiftshader-gn.patch
```

**[`0001-qnx-engine-build-toolchain-fml-txt.patch`](patches/0001-qnx-engine-build-toolchain-fml-txt.patch)** — QNX: build the embedder engine with a QNX clang toolchain.

> Upstream Flutter knows QNX only for the standalone Impeller library (qcc,
> arm64, no Dart, no embedder). This adds what the embedder engine needs:
> - //build/toolchain/qnx: clang toolchains for x64 and arm64 (qnx_use_clang,
>   qnx_clang_dir, qnx_sysroot_*), small compatibility headers for Linux names
>   QNX spells differently (endian.h, elf.h, dirent.h DT_*, sys/syscall.h for
>   gettid, sys/sendfile.h, sys/mman.h, sys/prctl.h, linux/random.h, fcntl.h
>   openat64), empty libdl/libpthread/librt (QNX has them in libc) and
>   -lsocket -lfsnotify -lcatalog;
> - the engine's own libc++ on QNX (the SDP's libc++ 18 has no std::atomic_ref),
>   libc++abi with exceptions, no _FORTIFY_SOURCE (QNX's headers use gcc builtins);
> - fml: a message loop on a condition variable (QNX has no epoll/timerfd);
> - txt: system fonts from Skia's FreeType directory font manager
>   (/usr/local/share/fonts), which also loads the fonts an app bundles;
> - Skia N32 = BGRA as on Linux (the byte order of wl_shm ARGB8888);
> - native assets: the target OS name is "linux";
> - the embedder engine and the snapshot tools in the build (flutter/BUILD.gn).

Touches: `engine/src/build/config/BUILDCONFIG.gn`, `engine/src/build/config/compiler/BUILD.gn`, `engine/src/build/toolchain/qnx/BUILD.gn`, `engine/src/build/toolchain/qnx/include/dirent.h`, `engine/src/build/toolchain/qnx/include/elf.h`, `engine/src/build/toolchain/qnx/include/endian.h` … · Upstream status: QNX-specific

**[`0002-qnx-dart-vm.patch`](patches/0002-qnx-dart-vm.patch)** — QNX: the Dart VM on QNX Neutrino.

> Dart treats QNX like FreeBSD: a Linux-family host (DART_HOST_OS_LINUX) with
> DART_HOST_OS_QNX for the differences, and the Linux target ABI for snapshots.
> New: a poll(2) event handler (bin/eventhandler_qnx.*; QNX has no epoll with
> edge triggering and no timerfd), a signal handler for QNX's mcontext layout;
> QNX ways for the thread id, random bytes (/dev/urandom), the thread's stack
> (TLS), fstatat, d_type, bzero and the machine-check codes.

Touches: `engine/src/flutter/third_party/dart/runtime/BUILD.gn`, `engine/src/flutter/third_party/dart/runtime/bin/crypto_linux.cc`, `engine/src/flutter/third_party/dart/runtime/bin/directory_linux.cc`, `engine/src/flutter/third_party/dart/runtime/bin/eventhandler.h`, `engine/src/flutter/third_party/dart/runtime/bin/eventhandler_linux.cc`, `engine/src/flutter/third_party/dart/runtime/bin/eventhandler_qnx.cc` … · Upstream status: QNX-specific

**[`0003-qnx-libcxx.patch`](patches/0003-qnx-libcxx.patch)** — QNX: libc++ for the engine build.

> The QNX parts of qnx-ports' LLVM (thread API, __locale masks, csetjmp,
> filesystem, errno) and a locale base API for QNX written here
> (__locale_dir/support/qnx.h, after Fuchsia's): QNX's C library has no
> locale_t, newlocale or the *_l functions, so libc++ supports the "C" locale
> only and implements wcsnrtombs/mbsnrtowcs itself.

Touches: `engine/src/flutter/third_party/libcxx/include/__config`, `engine/src/flutter/third_party/libcxx/include/__cxx03/__config`, `engine/src/flutter/third_party/libcxx/include/__cxx03/csetjmp`, `engine/src/flutter/third_party/libcxx/include/__locale`, `engine/src/flutter/third_party/libcxx/include/__locale_dir/locale_base_api.h`, `engine/src/flutter/third_party/libcxx/include/__locale_dir/support/qnx.h` … · Upstream status: QNX-specific

**[`0004-qnx-libcxxabi.patch`](patches/0004-qnx-libcxxabi.patch)** — QNX: libc++abi -- QNX in cxxabi.h's platform list (from qnx-ports' LLVM).

Touches: `engine/src/flutter/third_party/libcxxabi/include/cxxabi.h` · Upstream status: QNX-specific

**[`0005-qnx-boringssl-urandom.patch`](patches/0005-qnx-boringssl-urandom.patch)** — QNX: BoringSSL takes random bytes from /dev/urandom (QNX has no getentropy).

Touches: `engine/src/flutter/third_party/boringssl/src/crypto/rand/internal.h` · Upstream status: QNX-specific

**[`0006-qnx-skia-no-locale-t.patch`](patches/0006-qnx-skia-no-locale-t.patch)** — QNX: Skia's locale setter is a no-op as on Android (QNX has no locale_t).

Touches: `engine/src/flutter/third_party/skia/src/core/SkAutoLocaleSetter.h` · Upstream status: QNX-specific

**[`0007-qnx-zlib-no-arm-cpu-features.patch`](patches/0007-qnx-zlib-no-arm-cpu-features.patch)** — QNX: no ARM NEON/CRC32 code paths in zlib on QNX (they need Linux's

> getauxval/HWCAP to detect the CPU features).

Touches: `engine/src/flutter/third_party/zlib/BUILD.gn` · Upstream status: QNX-specific

**[`0008-qnx-swiftshader-gn.patch`](patches/0008-qnx-swiftshader-gn.patch)** — QNX arm64: SwiftShader's Reactor has no JIT backend there (no Subzero on

> arm64, LLVM not configured for QNX); the target stays declared so that the GN
> graph resolves -- nothing built for QNX links it.

Touches: `engine/src/flutter/third_party/swiftshader/src/Reactor/BUILD.gn` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/flutter-engine; B=build/$QNX_ARCH/flutter-engine; mkdir -p $B
```

```bash
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
```

```bash
do_build() {
  export PATH="$B/bin:$(_depot):$PATH"
  ninja -C "$S/engine/src/out/qnx_release_$(_cpu)" flutter/shell/platform/embedder:flutter_engine \
    flutter/lib/snapshot:generate_snapshot_bins
}
```

```bash
do_install() {
  local O="$S/engine/src/out/qnx_release_$(_cpu)"
  install -D -m 755 "$O/libflutter_engine.so" "$STAGE$PREFIX/lib/libflutter_engine.so"
  install -D -m 644 "$S/engine/src/flutter/shell/platform/embedder/embedder.h" "$STAGE$PREFIX/include/flutter_embedder.h"
  install -D -m 644 "$O/icudtl.dat" "$STAGE$PREFIX/share/flutter/icudtl.dat"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The Flutter engine for QNX: libflutter_engine.so (the embedder API, shell/platform/embedder/embedder.h), release mode, software rendering on the CPU (Skia raster; no GL/Vulkan yet). An embedder -- qnx-flutter-wayland, a Wayland window on labwc -- loads it together with an app's AOT snapshot. Upstream builds the engine only for its own platforms; it knows QNX just for the standalone Impeller library (qcc, arm64). patches/ add the embedder engine and the Dart VM for QNX (each patch says why): a QNX clang toolchain in the engine's GN build, Dart's QNX layer (a poll(2) event handler), the engine's libc++ with a "C"-locale base for QNX, fml's message loop, fonts from /usr/local/share/fonts. The source is Flutter's monorepo at the release tag plus its dependencies (gclient, depot_tools: about 28 GB). It is compiled with a build-host clang that knows QNX -- LLVM with the same qnx-ports patch as ports/llvm, built once into hosttools/qnx-clang -- against a sysroot made of the SDP's target headers and libraries. Snapshots of Dart code use the Linux target ABI, the same as QNX's on x86_64 and aarch64, so app AOT code (libapp.so) comes from this build's gen_snapshot (flutter-counter-example shows how).
- A build-host clang/lld with QNX support (qnx-ports), once for all components.
- The SDP's target headers and libraries in the layout clang's QNX driver expects (<sysroot>/usr/include, <sysroot>/usr/lib), plus the gcc runtime.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH flutter-engine`.

### 5. What it installs

3 files under `/usr/local`: 1 shared-library entries, 1 headers.

```
/usr/local/lib/libflutter_engine.so
```

The complete list is `stage/<arch>/.manifest/flutter-engine` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa flutter-engine
ls dists/8.0/*/ | grep '^flutter-engine-'
```

This writes `flutter-engine-<ver>-r0.tar.gz` (run time) and `flutter-engine-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add flutter-engine
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/flutter-engine-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/flutter-engine-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_flutter_engine`](../../tests/src/) links flutter-engine the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-flutter-engine`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-flutter-engine
/usr/local/bin/qnx-port-tests/test_flutter_engine
```

```
== flutter-engine ==
  ok   ...
PASS flutter-engine
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_flutter_engine` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_flutter_engine`](../../tests/src/) -- a program linked against flutter-engine the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - procedure table complete
   - release engine: runs AOT compiled Dart code
   - engine clock advances (20 ms sleep)
   - a missing AOT ELF is refused
   - the counter app's AOT ELF loads
   - engine runs the app (software renderer)
   - window metrics 320x240 accepted
   - frames presented within 15 s
   - frames have the window size 320x240
   - the app's surface is painted (light body colour in the middle)
   - engine shuts down

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> flutter-engine`.

## Versions

This recipe builds **3.50.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add flutter-engine=<version>`), sources in `ported/flutter-engine/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates flutter-engine`; port one with `scripts/port-update flutter-engine`.

<details><summary>recipe.sh</summary>

```bash
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
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
