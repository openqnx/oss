# thorvg 0.15.8

> Lightweight library for rendering vector graphics, SVG and Lottie

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/thorvg/thorvg.git (tag `v0.15.8`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`zlib`](../zlib/) |
| Run-time dependencies | [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_thorvg`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add thorvg
apk add thorvg-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-thorvg && /usr/local/bin/qnx-port-tests/test_thorvg    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install thorvg`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild thorvg` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (3 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libjpeg-turbo libpng zlib
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.15.8 --recurse-submodules --shallow-submodules \
    https://github.com/thorvg/thorvg.git src/thorvg
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/thorvg src/thorvg \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      --default-library=shared \
      -Dstatic=false \
      -Dloaders=svg,png,jpg,lottie \
      -Dsavers=gif \
      -Dtests=false \
      -Dexamples=false \
      -Dbindings=capi \
      -Dlog=false \
      -Dthreads=false
ninja -C build/$QNX_ARCH/thorvg
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/thorvg install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- --default-library=both (the driver's default) makes ThorVG define TVG_STATIC, which turns TVG_API into nothing at all -- the shared library then exports no tvg_* symbol and every consumer fails to link. Build shared only.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH thorvg`.

### 5. What it installs

7 files under `/usr/local`: 3 shared-library entries, 3 headers.

```
/usr/local/lib/libthorvg.so
/usr/local/lib/libthorvg.so.0
/usr/local/lib/libthorvg.so.0.15.8
```

The complete list is `stage/<arch>/.manifest/thorvg` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa thorvg
ls dists/8.0/*/ | grep '^thorvg-'
```

This writes `thorvg-<ver>-r0.tar.gz` (run time) and `thorvg-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add thorvg
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/thorvg-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/thorvg-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_thorvg`](../../tests/src/) links thorvg the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-thorvg`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-thorvg
/usr/local/bin/qnx-port-tests/test_thorvg
```

```
== thorvg ==
  ok   ...
PASS thorvg
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_thorvg` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_thorvg`](../../tests/src/) -- a program linked against thorvg the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - tvg_engine_init
   - allocate the canvas buffer
   - tvg_swcanvas_create
   - set the render target
   - tvg_shape_new
   - set the fill colour
   - push the shape
   - tvg_canvas_draw
   - tvg_canvas_sync
   - centre pixel was filled red
   - corner outside the shape is untouched

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | thorvg 0.15.8-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/thorvg-0.15.8-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | thorvg 0.15.8-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/thorvg-0.15.8-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.15.8**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add thorvg=<version>`), sources in `ported/thorvg/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates thorvg`; port one with `scripts/port-update thorvg`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.15.8; GIT=https://github.com/thorvg/thorvg.git; REF=v0.15.8
DEPS="libjpeg-turbo libpng zlib"
BUILDSYS=meson
# --default-library=both (the driver's default) makes ThorVG define TVG_STATIC,
# which turns TVG_API into nothing at all -- the shared library then exports no
# tvg_* symbol and every consumer fails to link. Build shared only.
MESON_ARGS_EXTRA=( --default-library=shared
                   -Dstatic=false
                   -Dloaders=svg,png,jpg,lottie -Dsavers=gif -Dtests=false
                   -Dexamples=false -Dbindings=capi -Dlog=false -Dthreads=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
