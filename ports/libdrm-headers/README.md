# libdrm-headers 2.4.129

> Headers for the DRM kernel interface and the libdrm API (headers only)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/mesa/drm.git (tag `libdrm-2.4.129`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libdrm_headers`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `libdrm-headers-dev` -- header-only: there is no run-time package, the -dev package is the whole component (headers, pkg-config/CMake files).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libdrm-headers-dev`](https://openqnx.github.io/oss/package/x86_64/libdrm-headers-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libdrm-headers-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libdrm-headers-dev` (dependencies come along); one version: `apk add libdrm-headers-dev=<version>` | `apk del libdrm-headers-dev` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libdrm-headers-dev` (dependencies first) | `oss-get remove libdrm-headers-dev` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-libdrm-headers && /usr/local/bin/qnx-port-tests/test_libdrm_headers    # its on-target test, from tests-8.0-<arch>
apk del libdrm-headers-dev qnx-test-libdrm-headers
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libdrm-headers-dev.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libdrm-headers-dev.list
rm var/lib/manual/libdrm-headers-dev.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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

### Test

[`test_libdrm_headers`](../../tests/src/) links libdrm-headers the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libdrm-headers`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libdrm-headers
/usr/local/bin/qnx-port-tests/test_libdrm_headers
```

```
== libdrm-headers ==
  ok   ...
PASS libdrm-headers
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libdrm_headers` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libdrm_headers`](../../tests/src/) -- a program linked against libdrm-headers the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - DRM_FORMAT_XRGB8888 = 'XR24' = 0x34325258
   - ARGB8888 'AR24', NV12, YUYV fourccs
   - modifiers LINEAR = 0, INVALID
   - I915_FORMAT_MOD_X_TILED: vendor Intel, code 1
   - DRM_FORMAT_BIG_ENDIAN flag
   - DRM_IOCTL_VERSION: group 'd', nr 0x00
   - MODE_GETRESOURCES nr 0xA0, MODE_ADDFB2 nr 0xB8
   - ioctl encodes sizeof(struct drm_mode_card_res)
   - modeinfo 68 bytes, card_res 64 bytes
   - drm_mode_fb_cmd2: pixel_format at 12, modifier[] at 72
   - connector types, mode flags
   - DRM_CAP_DUMB_BUFFER, DRM_CLIENT_CAP_ATOMIC
   - a 1080p60 mode line: refresh 60 Hz

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libdrm-headers 2.4.129-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libdrm-headers-2.4.129-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libdrm-headers 2.4.129-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libdrm-headers-2.4.129-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libdrm-headers 2.4.129-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libdrm-headers-2.4.129-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libdrm-headers 2.4.129-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libdrm-headers-2.4.129-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

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
