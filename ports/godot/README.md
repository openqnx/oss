# godot 4.4.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/godotengine/godot.git (tag `4.4.1-stable`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`freetype`](../freetype/) [`harfbuzz`](../harfbuzz/) [`graphite2`](../graphite2/) [`libogg`](../libogg/) [`libtheora`](../libtheora/) [`libpng`](../libpng/) [`libvorbis`](../libvorbis/) [`libwebp`](../libwebp/) [`opus`](../opus/) [`pcre2`](../pcre2/) [`zlib`](../zlib/) [`zstd`](../zstd/) [`brotli`](../brotli/) [`libxkbcommon`](../libxkbcommon/) |
| Run-time dependencies | [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`libiconv`](../libiconv/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libtheora`](../libtheora/) [`libvorbis`](../libvorbis/) [`libwebp`](../libwebp/) [`libxkbcommon`](../libxkbcommon/) [`pcre2`](../pcre2/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add godot
apk add godot-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install godot`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild godot` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (26 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild freetype harfbuzz graphite2 libogg libtheora libpng libvorbis libwebp opus pcre2 zlib zstd brotli libxkbcommon
```

<details><summary>Full build chain (26)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libtheora`](../libtheora/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libwebp`](../libwebp/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`opus`](../opus/) [`pcre2`](../pcre2/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 4.4.1-stable --recurse-submodules --shallow-submodules \
    https://github.com/godotengine/godot.git src/godot
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/godot < ports/godot/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — [Core] Fix performance issues in some `CowData` methods

> Imported from qnx-ports (godotengine/godot 4.4.1-stable...qnx-4.4.1-r1); original authorship kept.
>
>
> These relied on `get` which does a bounds check each time on debug
> builds
>
> ---
>  core/templates/cowdata.h | 24 ++++++++++++------------
>  1 file changed, 12 insertions(+), 12 deletions(-)

Touches: `core/templates/cowdata.h`, `modules/text_server_adv/text_server_adv.cpp`, `modules/text_server_fb/text_server_fb.cpp`, `core/io/file_access_compressed.cpp`, `servers/audio/effects/audio_effect_pitch_shift.cpp`, `misc/scripts/install_vulkan_sdk_macos.sh` … · Upstream status: QNX-specific (from qnx-ports)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/godot; B=build/$QNX_ARCH/godot; mkdir -p $B
```

```bash
do_build() {
  # SCons in a venv of its own: the build machine's Python may refuse a pip
  # install outside one ("externally-managed-environment").
  local v="$BUILD_DIR/host/scons-venv"
  [ -x "$v/bin/scons" ] || { python3 -m venv "$v" && "$v/bin/pip" install -q scons; } || return 1
  _gd target=template_release && _gd target=template_debug
}
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/bin" && \
  install -m755 "$S"/bin/godot.qnx.template_release.* "$STAGE$PREFIX/bin/godot-template-release-qnxscreen" && \
  install -m755 "$S"/bin/godot.qnx.template_debug.* "$STAGE$PREFIX/bin/godot-template-debug-qnxscreen" && \
  rm -f "$S"/bin/godot.qnx.*
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Godot 4.4.1 with qnx-ports' QNX support (patches/: the godot-QNX fork's commits on 4.4.1-stable -- a "qnx" platform on Screen/EGL/GLES3, QNX audio and input). Built here: the QNX export templates (template_release, template_debug), i.e. the runtime that runs Godot games on a QNX target, cross compiled with the SDP toolchain. The QNX Everywhere image also has the Godot editor; that one is built natively on QNX with clang (linuxbsd/Wayland platform) and is not cross-buildable this way. SCons options follow QNX's aports (extra/godot/APKBUILD), using this tree's libraries instead of the bundled copies.
- import_env_vars: SCons runs pkg-config (ParseConfig) in its own minimal environment, which then answered from the build machine's .pc files -- host freetype2 flags, no theora at all. Hand it the tree's pkg-config setup.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH godot`.

### 5. What it installs

2 files under `/usr/local`, 2 programs.

```
/usr/local/bin/godot-template-debug-qnxscreen
/usr/local/bin/godot-template-release-qnxscreen
```

The complete list is `stage/<arch>/.manifest/godot` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa godot
ls dists/8.0/*/ | grep '^godot-'
```

This writes `godot-<ver>-r0.tar.gz` (run time) and `godot-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add godot
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/godot-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/godot-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | godot 4.4.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/godot-4.4.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | godot 4.4.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/godot-4.4.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.4.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add godot=<version>`), sources in `ported/godot/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates godot`; port one with `scripts/port-update godot`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.4.1; GIT=https://github.com/godotengine/godot.git; REF=4.4.1-stable
# Godot 4.4.1 with qnx-ports' QNX support (patches/: the godot-QNX fork's
# commits on 4.4.1-stable -- a "qnx" platform on Screen/EGL/GLES3, QNX audio
# and input). Built here: the QNX export templates (template_release,
# template_debug), i.e. the runtime that runs Godot games on a QNX target,
# cross compiled with the SDP toolchain. The QNX Everywhere image also has the
# Godot editor; that one is built natively on QNX with clang (linuxbsd/Wayland
# platform) and is not cross-buildable this way.
# SCons options follow QNX's aports (extra/godot/APKBUILD), using this tree's
# libraries instead of the bundled copies.
DEPS="freetype harfbuzz graphite2 libogg libtheora libpng libvorbis libwebp opus pcre2 zlib zstd brotli libxkbcommon"
BUILDSYS=custom
_gd() {
  cd "$S" && PKG_CONFIG_LIBDIR="$PKG_CONFIG_LIBDIR" PKG_CONFIG_SYSROOT_DIR="$PKG_CONFIG_SYSROOT_DIR" \
  "$BUILD_DIR/host/scons-venv/bin/python" -m SCons -j"$(nproc)" platform=qnx arch="$([ "$QNX_CPU" = aarch64 ] && echo arm64 || echo x86_64)" \
    module_camera_enabled=false builtin_brotli=false builtin_freetype=false builtin_graphite=false builtin_harfbuzz=false \
    builtin_libogg=false builtin_libtheora=false builtin_libpng=false builtin_libvorbis=false builtin_libwebp=false \
    builtin_opus=false builtin_pcre2=false builtin_zlib=false builtin_zstd=false engine_update_check=false execinfo=false \
    lto=none production=true system_certs_path=/usr/local/ssl/cert.pem use_sowrap=false use_static_cpp=false \
    module_denoise_enabled=false import_env_vars=PKG_CONFIG_LIBDIR,PKG_CONFIG_SYSROOT_DIR,PKG_CONFIG_PATH "$@"
}
# import_env_vars: SCons runs pkg-config (ParseConfig) in its own minimal
# environment, which then answered from the build machine's .pc files --
# host freetype2 flags, no theora at all. Hand it the tree's pkg-config setup.
do_build() {
  # SCons in a venv of its own: the build machine's Python may refuse a pip
  # install outside one ("externally-managed-environment").
  local v="$BUILD_DIR/host/scons-venv"
  [ -x "$v/bin/scons" ] || { python3 -m venv "$v" && "$v/bin/pip" install -q scons; } || return 1
  _gd target=template_release && _gd target=template_debug
}
do_install() {
  install -d "$STAGE$PREFIX/bin" && \
  install -m755 "$S"/bin/godot.qnx.template_release.* "$STAGE$PREFIX/bin/godot-template-release-qnxscreen" && \
  install -m755 "$S"/bin/godot.qnx.template_debug.* "$STAGE$PREFIX/bin/godot-template-debug-qnxscreen" && \
  rm -f "$S"/bin/godot.qnx.*
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
