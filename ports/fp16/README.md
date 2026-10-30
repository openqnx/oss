# fp16 0-git-20260620

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/Maratyszcza/FP16.git |
| Build system | CMake |
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
apk add fp16
apk add fp16-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install fp16`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild fp16` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/Maratyszcza/FP16.git src/fp16
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/fp16 -B build/$QNX_ARCH/fp16 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DFP16_BUILD_TESTS=OFF \
      -DFP16_BUILD_BENCHMARKS=OFF
cmake --build build/$QNX_ARCH/fp16 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/fp16
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Header-only half-precision conversions (used by XNNPACK). No releases.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH fp16`.

### 5. What it installs

4 files under `/usr/local`, 4 headers.

The complete list is `stage/<arch>/.manifest/fp16` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa fp16
ls dists/8.0/*/ | grep '^fp16-'
```

This writes `fp16-<ver>-r0.tar.gz` (run time) and `fp16-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add fp16
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/fp16-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/fp16-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | fp16 0_git20260620-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | fp16 0_git20260620-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **0-git-20260620**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add fp16=<version>`), sources in `ported/fp16/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates fp16`; port one with `scripts/port-update fp16`.

<details><summary>recipe.sh</summary>

```bash
# Header-only half-precision conversions (used by XNNPACK). No releases.
VERSION=0-git-20260620; GIT=https://github.com/Maratyszcza/FP16.git; COMMIT=782eea126dc5c755827be751a099eb01826175cf
CMAKE_ARGS_EXTRA=( -DFP16_BUILD_TESTS=OFF -DFP16_BUILD_BENCHMARKS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
