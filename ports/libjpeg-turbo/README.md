# libjpeg-turbo 3.0.4

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/libjpeg-turbo/libjpeg-turbo.git (tag `3.0.4`) |
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
apk add libjpeg-turbo
apk add libjpeg-turbo-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libjpeg-turbo`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libjpeg-turbo` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 3.0.4 --recurse-submodules --shallow-submodules \
    https://github.com/libjpeg-turbo/libjpeg-turbo.git src/libjpeg-turbo
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libjpeg-turbo -B build/$QNX_ARCH/libjpeg-turbo -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DENABLE_SHARED=ON \
      -DENABLE_STATIC=ON \
      -DWITH_TURBOJPEG=ON \
      -DWITH_JAVA=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/libjpeg-turbo -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libjpeg-turbo
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libjpeg-turbo`.

### 5. What it installs

39 files under `/usr/local`: 6 shared-library entries, 6 programs, 5 headers.

```
/usr/local/lib/libjpeg.so
/usr/local/lib/libjpeg.so.62
/usr/local/lib/libjpeg.so.62.4.0
/usr/local/lib/libturbojpeg.so
/usr/local/lib/libturbojpeg.so.0
/usr/local/lib/libturbojpeg.so.0.3.0
/usr/local/bin/cjpeg
/usr/local/bin/djpeg
/usr/local/bin/jpegtran
/usr/local/bin/rdjpgcom
/usr/local/bin/tjbench
/usr/local/bin/wrjpgcom
```

The complete list is `stage/<arch>/.manifest/libjpeg-turbo` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libjpeg-turbo
ls dists/8.0/*/ | grep '^libjpeg-turbo-'
```

This writes `libjpeg-turbo-<ver>-r0.tar.gz` (run time) and `libjpeg-turbo-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libjpeg-turbo
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libjpeg-turbo-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libjpeg-turbo-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libjpeg-turbo 3.0.4-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libjpeg-turbo 3.0.4-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **3.0.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libjpeg-turbo=<version>`), sources in `ported/libjpeg-turbo/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libjpeg-turbo`; port one with `scripts/port-update libjpeg-turbo`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.0.4; GIT=https://github.com/libjpeg-turbo/libjpeg-turbo.git; REF=3.0.4
CMAKE_ARGS_EXTRA=( -DENABLE_SHARED=ON -DENABLE_STATIC=ON -DWITH_TURBOJPEG=ON
                   -DWITH_JAVA=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
