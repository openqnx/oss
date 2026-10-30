# libdrm-headers 2.4.129

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/mesa/drm.git (tag `libdrm-2.4.129`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libdrm-headers
apk add libdrm-headers-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libdrm-headers`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libdrm-headers` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b libdrm-2.4.129 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/mesa/drm.git src/libdrm-headers
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/libdrm-headers; B=build/$QNX_ARCH/libdrm-headers; mkdir -p $B
```

```bash
do_configure() { :; }
```

```bash
do_build() { :; }
```

```bash
do_install() {
  local d="$STAGE$PREFIX/include/libdrm-uapi"; mkdir -p "$d"
  install -m644 "$S"/include/drm/*.h "$S"/xf86drm.h "$S"/xf86drmMode.h "$d/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libdrm-headers: libdrm 2.4.129's headers, header only -- the DRM kernel interface (drm.h, drm_fourcc.h, drm_mode.h, ...) and the library's own (xf86drm.h, xf86drmMode.h). The library is the QNX SDP's libdrm.so, which exports the newer functions (drmGetFormatName, drmModeCreateDumbBuffer, drmSyncobjEventfd, ...) but ships older headers that declare none of them, nor DRM_FORMAT_R16F and the other formats wlroots 0.20 uses. Installed to include/libdrm-uapi, ahead of the SDP's on the include path (scripts/lib/sdp-pc.sh's libdrm.pc).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libdrm-headers`.

### 5. What it installs

22 files under `/usr/local`, 22 headers.

The complete list is `stage/<arch>/.manifest/libdrm-headers` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libdrm-headers
ls dists/8.0/*/ | grep '^libdrm-headers-'
```

This writes `libdrm-headers-<ver>-r0.tar.gz` (run time) and `libdrm-headers-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libdrm-headers
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libdrm-headers-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libdrm-headers-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libdrm-headers 2.4.129-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libdrm-headers 2.4.129-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **2.4.129**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libdrm-headers=<version>`), sources in `ported/libdrm-headers/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libdrm-headers`; port one with `scripts/port-update libdrm-headers`.

<details><summary>recipe.sh</summary>

```bash
# libdrm-headers: libdrm 2.4.129's headers, header only -- the DRM kernel
# interface (drm.h, drm_fourcc.h, drm_mode.h, ...) and the library's own
# (xf86drm.h, xf86drmMode.h). The library is the QNX SDP's libdrm.so, which
# exports the newer functions (drmGetFormatName, drmModeCreateDumbBuffer,
# drmSyncobjEventfd, ...) but ships older headers that declare none of them,
# nor DRM_FORMAT_R16F and the other formats wlroots 0.20 uses. Installed to
# include/libdrm-uapi, ahead of the SDP's on the include path
# (scripts/lib/sdp-pc.sh's libdrm.pc).
VERSION=2.4.129; GIT=https://gitlab.freedesktop.org/mesa/drm.git; REF=libdrm-2.4.129
DEPS=""
BUILDSYS=custom
do_configure() { :; }
do_build() { :; }
do_install() {
  local d="$STAGE$PREFIX/include/libdrm-uapi"; mkdir -p "$d"
  install -m644 "$S"/include/drm/*.h "$S"/xf86drm.h "$S"/xf86drmMode.h "$d/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
