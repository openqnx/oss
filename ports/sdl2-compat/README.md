# sdl2-compat 2.32.60

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/libsdl-org/sdl2-compat.git (tag `release-2.32.60`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`sdl3`](../sdl3/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add sdl2-compat
apk add sdl2-compat-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install sdl2-compat`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild sdl2-compat` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (25 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild sdl3
```

<details><summary>Full build chain (25)</summary>

[`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcursor`](../libxcursor/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxi`](../libxi/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxscrnsaver`](../libxscrnsaver/) [`libxtst`](../libxtst/) [`ncurses`](../ncurses/) [`pthread-stubs`](../pthread-stubs/) [`sdl3`](../sdl3/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b release-2.32.60 --recurse-submodules --shallow-submodules \
    https://github.com/libsdl-org/sdl2-compat.git src/sdl2-compat
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/sdl2-compat -B build/$QNX_ARCH/sdl2-compat -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DSDL2COMPAT_TESTS=OFF \
      -DSDL2COMPAT_INSTALL_TESTS=OFF
cmake --build build/$QNX_ARCH/sdl2-compat -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/sdl2-compat
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- sdl2-compat: the SDL2 API implemented on top of SDL3 (the image's SDL2). Installs libSDL2-2.0 like the sdl2 component, so use one or the other.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH sdl2-compat`.

### 5. What it installs

112 files under `/usr/local`: 4 shared-library entries, 1 programs, 92 headers.

```
/usr/local/lib/libSDL2-2.0.so
/usr/local/lib/libSDL2-2.0.so.0
/usr/local/lib/libSDL2-2.0.so.0.3200.60
/usr/local/lib/libSDL2.so
/usr/local/bin/sdl2-config
```

The complete list is `stage/<arch>/.manifest/sdl2-compat` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa sdl2-compat
ls dists/8.0/*/ | grep '^sdl2-compat-'
```

This writes `sdl2-compat-<ver>-r0.tar.gz` (run time) and `sdl2-compat-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add sdl2-compat
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/sdl2-compat-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/sdl2-compat-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | sdl2-compat 2.32.60-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | sdl2-compat 2.32.60-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **2.32.60**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add sdl2-compat=<version>`), sources in `ported/sdl2-compat/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates sdl2-compat`; port one with `scripts/port-update sdl2-compat`.

<details><summary>recipe.sh</summary>

```bash
# sdl2-compat: the SDL2 API implemented on top of SDL3 (the image's SDL2).
# Installs libSDL2-2.0 like the sdl2 component, so use one or the other.
VERSION=2.32.60; GIT=https://github.com/libsdl-org/sdl2-compat.git; REF=release-2.32.60
DEPS="sdl3"
CONFLICTS="sdl2"
CMAKE_ARGS_EXTRA=( -DSDL2COMPAT_TESTS=OFF -DSDL2COMPAT_INSTALL_TESTS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
