# lcms2 2.16

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/mm2/Little-CMS.git (tag `lcms2.16`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_lcms2`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add lcms2
apk add lcms2-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-lcms2 && /usr/local/bin/qnx-port-tests/test_lcms2    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install lcms2`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild lcms2` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b lcms2.16 --recurse-submodules --shallow-submodules \
    https://github.com/mm2/Little-CMS.git src/lcms2
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/lcms2 src/lcms2 \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dutils=false \
      -Djpeg=disabled \
      -Dtiff=disabled
ninja -C build/$QNX_ARCH/lcms2
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/lcms2 install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH lcms2`.

### 5. What it installs

7 files under `/usr/local`: 3 shared-library entries, 2 headers.

```
/usr/local/lib/liblcms2.so
/usr/local/lib/liblcms2.so.2
/usr/local/lib/liblcms2.so.2.0.16
```

The complete list is `stage/<arch>/.manifest/lcms2` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa lcms2
ls dists/8.0/*/ | grep '^lcms2-'
```

This writes `lcms2-<ver>-r0.tar.gz` (run time) and `lcms2-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add lcms2
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/lcms2-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/lcms2-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_lcms2`](../../tests/src/) links lcms2 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-lcms2`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-lcms2
/usr/local/bin/qnx-port-tests/test_lcms2
```

```
== lcms2 ==
  ok   ...
PASS lcms2
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_lcms2` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_lcms2`](../../tests/src/) -- a program linked against lcms2 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - create sRGB and Lab profiles
   - cmsCreateTransform sRGB -> Lab
   - white maps to L* ~ 100
   - white is neutral in a*/b*
   - black maps to L* ~ 0
   - reverse transform
   - Lab red maps back to sRGB red

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **2.16**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add lcms2=<version>`), sources in `ported/lcms2/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates lcms2`; port one with `scripts/port-update lcms2`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.16; GIT=https://github.com/mm2/Little-CMS.git; REF=lcms2.16
DEPS="zlib"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dutils=false -Djpeg=disabled -Dtiff=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
