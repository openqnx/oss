# dav1d 1.5.0

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://code.videolan.org/videolan/dav1d.git (tag `1.5.0`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_dav1d`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add dav1d
apk add dav1d-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-dav1d && /usr/local/bin/qnx-port-tests/test_dav1d    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install dav1d`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild dav1d` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 1.5.0 --recurse-submodules --shallow-submodules \
    https://code.videolan.org/videolan/dav1d.git src/dav1d
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/dav1d src/dav1d \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Denable_tools=false \
      -Denable_tests=false \
      -Denable_examples=false
ninja -C build/$QNX_ARCH/dav1d
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/dav1d install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH dav1d`.

### 5. What it installs

11 files under `/usr/local`: 3 shared-library entries, 6 headers.

```
/usr/local/lib/libdav1d.so
/usr/local/lib/libdav1d.so.7
/usr/local/lib/libdav1d.so.7.0.0
```

The complete list is `stage/<arch>/.manifest/dav1d` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa dav1d
ls dists/8.0/*/ | grep '^dav1d-'
```

This writes `dav1d-<ver>-r0.tar.gz` (run time) and `dav1d-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add dav1d
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/dav1d-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/dav1d-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_dav1d`](../../tests/src/) links dav1d the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-dav1d`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-dav1d
/usr/local/bin/qnx-port-tests/test_dav1d
```

```
== dav1d ==
  ok   ...
PASS dav1d
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_dav1d` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_dav1d`](../../tests/src/) -- a program linked against dav1d the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - settings are writable
   - dav1d_open
   - dav1d_data_wrap
   - decoder accepts a temporal delimiter OBU
   - no picture yet (more data needed)
   - dav1d_close cleared the context

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | dav1d 1.5.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/dav1d-1.5.0-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | dav1d 1.5.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/dav1d-1.5.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.5.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add dav1d=<version>`), sources in `ported/dav1d/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates dav1d`; port one with `scripts/port-update dav1d`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.5.0; GIT=https://code.videolan.org/videolan/dav1d.git; REF=1.5.0
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Denable_tools=false -Denable_tests=false -Denable_examples=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
