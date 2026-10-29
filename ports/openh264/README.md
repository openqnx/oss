# openh264 2.5.0

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/cisco/openh264.git (tag `v2.5.0`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_openh264`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add openh264
apk add openh264-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-openh264 && /usr/local/bin/qnx-port-tests/test_openh264    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install openh264`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild openh264` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2.5.0 --recurse-submodules --shallow-submodules \
    https://github.com/cisco/openh264.git src/openh264
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/openh264 < ports/openh264/patches/0001-qnx-elf-system.patch
```

**[`0001-qnx-elf-system.patch`](patches/0001-qnx-elf-system.patch)** — openh264: treat QNX like Linux in meson.build.

> QNX is an ELF platform built with GNU as; meson.build otherwise stops at
> "FIXME: Unhandled system qnx".

Touches: `meson.build` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/openh264 src/openh264 \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=disabled
ninja -C build/$QNX_ARCH/openh264
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/openh264 install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH openh264`.

### 5. What it installs

9 files under `/usr/local`: 3 shared-library entries, 4 headers.

```
/usr/local/lib/libopenh264.so
/usr/local/lib/libopenh264.so.2.5.0
/usr/local/lib/libopenh264.so.7
```

The complete list is `stage/<arch>/.manifest/openh264` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa openh264
ls dists/8.0/*/ | grep '^openh264-'
```

This writes `openh264-<ver>-r0.tar.gz` (run time) and `openh264-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add openh264
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/openh264-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/openh264-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_openh264`](../../tests/src/) links openh264 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-openh264`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-openh264
/usr/local/bin/qnx-port-tests/test_openh264
```

```
== openh264 ==
  ok   ...
PASS openh264
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_openh264` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_openh264`](../../tests/src/) -- a program linked against openh264 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - WelsCreateSVCEncoder
   - encoder Initialize
   - WelsCreateDecoder
   - decoder Initialize
   - frames encoded
   - at least one frame decoded back

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **2.5.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add openh264=<version>`), sources in `ported/openh264/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates openh264`; port one with `scripts/port-update openh264`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.5.0; GIT=https://github.com/cisco/openh264.git; REF=v2.5.0
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
