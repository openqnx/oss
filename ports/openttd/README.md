# openttd 15.0-git-20250501

> OpenTTD, an open source simulation game based on Transport Tycoon Deluxe

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/OpenTTD/OpenTTD.git |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | [`sdl2`](../sdl2/) [`zlib`](../zlib/) [`xz`](../xz/) [`libpng`](../libpng/) [`freetype`](../freetype/) [`fontconfig`](../fontconfig/) [`curl`](../curl/) [`lzo`](../lzo/) |
| Run-time dependencies | [`curl`](../curl/) [`freetype`](../freetype/) [`libpng`](../libpng/) [`lzo`](../lzo/) [`sdl2`](../sdl2/) [`xz`](../xz/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add openttd
apk add openttd-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install openttd`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild openttd` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (23 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild sdl2 zlib xz libpng freetype fontconfig curl lzo
```

<details><summary>Full build chain (23)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`curl`](../curl/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`openssl`](../openssl/) [`sdl2`](../sdl2/) [`wayland`](../wayland/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/OpenTTD/OpenTTD.git src/openttd
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/openttd < ports/openttd/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/openttd < ports/openttd/patches/0002-qnx-link-libpng.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — QNX Changes

> Imported from qnx-ports/openttd (41b113840a98...qnx_master); original authorship kept.
>
>
> ---
>  CMakeLists.txt                | 27 ++++++++++++++++++++++++++-
>  cmake/CompileFlags.cmake      |  7 +++++--
>  src/3rdparty/catch2/catch.hpp |  2 ++
>  src/music/CMakeLists.txt      |  2 +-
>  src/os/unix/crashlog_unix.cpp |  7 ++++++-
>  5 files changed, 40 insertions(+), 5 deletions(-)

Touches: `CMakeLists.txt`, `cmake/CompileFlags.cmake`, `src/3rdparty/catch2/catch.hpp`, `src/music/CMakeLists.txt`, `src/os/unix/crashlog_unix.cpp` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-link-libpng.patch`](patches/0002-qnx-link-libpng.patch)** — Link libpng on QNX as everywhere else.

> The QNX patch (0001) leaves link_package(PNG) out on QNX, but the PNG
> screenshot and heightmap code is still compiled whenever FindPNG succeeds,
> and the openttd link then fails: "undefined reference to symbol
> png_write_row@@PNG16_0" (DSO missing from command line). This tree has
> libpng (the libpng component); link it.

Touches: `CMakeLists.txt` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/openttd; B=build/$QNX_ARCH/openttd; mkdir -p $B
```

```bash
do_configure() {
  local nat="$B/native"; mkdir -p "$nat"
  env -u CC -u CXX -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR cmake -S "$S" -B "$nat" -G Ninja -DOPTION_TOOLS_ONLY=ON && \
    cmake --build "$nat" --target tools || return 1
  # C++20 for every target: the toolchain's default flags say -std=gnu++17,
  # which openttd_test (no standard of its own) kept, and std::source_location
  # is C++20 ("source_location in namespace std does not name a type").
  cmake -S "$S" -B "$B/target" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DOPTION_TOOLS_ONLY=OFF -DHOST_BINARY_DIR="$nat" \
    -DCMAKE_DISABLE_FIND_PACKAGE_ICU=ON -DCPU="$QNX_CPU"
}
```

```bash
do_build() { cmake --build "$B/target" -j"$(nproc)"; }
```

```bash
do_install() { DESTDIR="$STAGE" cmake --install "$B/target"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- OpenTTD (open source Transport Tycoon Deluxe), at the upstream commit qnx-ports' port is based on (patches/: QNX support); part of qnx-ports' RetroPie set. OpenTTD generates sources with tools it builds first, so a native build of those tools is made (HOST_BINARY_DIR).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH openttd`.

### 5. What it installs

153 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/openttd` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa openttd
ls dists/8.0/*/ | grep '^openttd-'
```

This writes `openttd-<ver>-r0.tar.gz` (run time) and `openttd-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add openttd
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/openttd-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/openttd-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | openttd 15.0_git20250501-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/openttd-15.0_git20250501-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | openttd 15.0_git20250501-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/openttd-15.0_git20250501-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **15.0-git-20250501**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add openttd=<version>`), sources in `ported/openttd/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates openttd`; port one with `scripts/port-update openttd`.

<details><summary>recipe.sh</summary>

```bash
# OpenTTD (open source Transport Tycoon Deluxe), at the upstream commit
# qnx-ports' port is based on (patches/: QNX support); part of qnx-ports'
# RetroPie set. OpenTTD generates sources with tools it builds first, so a
# native build of those tools is made (HOST_BINARY_DIR).
VERSION=15.0-git-20250501; GIT=https://github.com/OpenTTD/OpenTTD.git; COMMIT=41b113840a983e2e9b107a452f800eb94c662b23
DEPS="sdl2 zlib xz libpng freetype fontconfig curl lzo"
BUILDSYS=custom
do_configure() {
  local nat="$B/native"; mkdir -p "$nat"
  env -u CC -u CXX -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR cmake -S "$S" -B "$nat" -G Ninja -DOPTION_TOOLS_ONLY=ON && \
    cmake --build "$nat" --target tools || return 1
  # C++20 for every target: the toolchain's default flags say -std=gnu++17,
  # which openttd_test (no standard of its own) kept, and std::source_location
  # is C++20 ("source_location in namespace std does not name a type").
  cmake -S "$S" -B "$B/target" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DOPTION_TOOLS_ONLY=OFF -DHOST_BINARY_DIR="$nat" \
    -DCMAKE_DISABLE_FIND_PACKAGE_ICU=ON -DCPU="$QNX_CPU"
}
do_build() { cmake --build "$B/target" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B/target"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
