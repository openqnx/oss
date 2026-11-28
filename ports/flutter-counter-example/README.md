# flutter-counter-example 1.0

> Flutter's counter example app for QNX (runs with qnx-flutter-wayland)

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`flutter-engine`](../flutter-engine/) [`qnx-flutter-wayland`](../qnx-flutter-wayland/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_flutter_counter_example`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `flutter-counter-example` and `flutter-counter-example-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild flutter-counter-example` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (20 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild flutter-engine qnx-flutter-wayland
```

<details><summary>Full build chain (20)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`flutter-engine`](../flutter-engine/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`qnx-flutter-wayland`](../qnx-flutter-wayland/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

```bash
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/flutter-counter-example/files/." "$SRCROOT/"; }
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/flutter-counter-example; B=build/$QNX_ARCH/flutter-counter-example; mkdir -p $B
```

```bash
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
```

```bash
do_install() {
  local D="$STAGE$PREFIX/share/flutter-apps/counter"
  mkdir -p "$D" && cp -a "$B/bundle/." "$D/" && \
  install -Dm755 "$S/flutter-counter" "$STAGE$PREFIX/bin/flutter-counter" && \
  sed -i "s|@PREFIX@|$PREFIX|g" "$STAGE$PREFIX/bin/flutter-counter" && \
  install -Dm644 "$S/flutter-counter.desktop" "$STAGE$PREFIX/share/applications/flutter-counter.desktop"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- flutter-counter-example: Flutter's own "counter" starter app (flutter create) built for QNX -- the release AOT snapshot (libapp.so) and assets, run by qnx-flutter-wayland: `flutter-counter` opens it on the desktop. How a Flutter app is built for QNX (the same for any app): 1. flutter build bundle --release   -> flutter_assets/ (fonts, images, ...) 2. the frontend server of the Flutter checkout compiles the Dart code to a kernel file (AOT, tree shaken) against the engine's flutter_patched_sdk; 3. gen_snapshot of the QNX engine build (a build-host program targeting the QNX CPU) turns it into libapp.so (app-aot-elf). Both the Flutter tool and the engine build come from flutter-engine's tree.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH flutter-counter-example`.

### 5. What it installs

13 files under `/usr/local`, 1 programs.

```
/usr/local/bin/flutter-counter
```

The complete list is `stage/<arch>/.manifest/flutter-counter-example` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa flutter-counter-example
ls dists/8.0/*/ | grep '^flutter-counter-example-'
```

This writes `flutter-counter-example-<ver>-r0.tar.gz` (run time) and `flutter-counter-example-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add flutter-counter-example
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/flutter-counter-example-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/flutter-counter-example-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_flutter_counter_example`](../../tests/src/) links flutter-counter-example the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-flutter-counter-example`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-flutter-counter-example
/usr/local/bin/qnx-port-tests/test_flutter_counter_example
```

```
== flutter-counter-example ==
  ok   ...
PASS flutter-counter-example
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_flutter_counter_example` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_flutter_counter_example`](../../tests/src/) -- a program linked against flutter-counter-example the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> flutter-counter-example`.

## Versions

This recipe builds **1.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add flutter-counter-example=<version>`), sources in `ported/flutter-counter-example/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates flutter-counter-example`; port one with `scripts/port-update flutter-counter-example`.

<details><summary>recipe.sh</summary>

```bash
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
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
