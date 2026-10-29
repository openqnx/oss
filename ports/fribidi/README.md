# fribidi 1.0.16

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/fribidi/fribidi.git (tag `v1.0.16`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add fribidi
apk add fribidi-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install fribidi`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild fribidi` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v1.0.16 --recurse-submodules --shallow-submodules \
    https://github.com/fribidi/fribidi.git src/fribidi
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/fribidi < ports/fribidi/patches/0001-qnx-no-ansi-c90.patch
```

**[`0001-qnx-no-ansi-c90.patch`](patches/0001-qnx-no-ansi-c90.patch)** — fribidi: do not build in C90 (-ansi) mode on QNX.

> -ansi means C90, where "inline" is not a keyword, and QNX's own headers use
> "static inline" (stdlib.h declares atexit that way): the build failed inside
> the system headers.

Touches: `meson.build` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/fribidi src/fribidi \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Ddocs=false \
      -Dbin=false \
      -Dtests=false
ninja -C build/$QNX_ARCH/fribidi
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/fribidi install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH fribidi`.

### 5. What it installs

27 files under `/usr/local`: 3 shared-library entries, 22 headers.

```
/usr/local/lib/libfribidi.so
/usr/local/lib/libfribidi.so.0
/usr/local/lib/libfribidi.so.0.4.0
```

The complete list is `stage/<arch>/.manifest/fribidi` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa fribidi
ls dists/8.0/*/ | grep '^fribidi-'
```

This writes `fribidi-<ver>-r0.tar.gz` (run time) and `fribidi-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add fribidi
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/fribidi-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/fribidi-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | fribidi 1.0.16-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | fribidi 1.0.16-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.0.16**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add fribidi=<version>`), sources in `ported/fribidi/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates fribidi`; port one with `scripts/port-update fribidi`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.0.16; GIT=https://github.com/fribidi/fribidi.git; REF=v1.0.16
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Ddocs=false -Dbin=false -Dtests=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
