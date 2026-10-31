# gklib 5.1.1-git-20230326

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/KarypisLab/GKlib.git |
| Build system | CMake |
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
apk add gklib
apk add gklib-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install gklib`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gklib` performs exactly the same steps (2–5) in one command.

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
    https://github.com/KarypisLab/GKlib.git src/gklib
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/gklib < ports/gklib/patches/0001-qnx-from-qnx-ports-build-files.patch
```

**[`0001-qnx-from-qnx-ports-build-files.patch`](patches/0001-qnx-from-qnx-ports-build-files.patch)** — Build GKlib for QNX: no tests by default, no -march=native, QNX string functions.

> Taken from qnx-ports/build-files ports/METIS/patches/GKlib.patch.

Touches: `CMakeLists.txt`, `GKlibSystem.cmake`, `string.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/gklib -B build/$QNX_ARCH/gklib -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DOPENMP=OFF
cmake --build build/$QNX_ARCH/gklib -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/gklib
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Pinned to the GKlib commit qnx-ports builds METIS against. GKlib: the support library METIS is built on (same authors). No releases.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gklib`.

### 5. What it installs

21 files under `/usr/local`: 1 shared-library entries, 20 headers.

```
/usr/local/lib/libGKlib.so
```

The complete list is `stage/<arch>/.manifest/gklib` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gklib
ls dists/8.0/*/ | grep '^gklib-'
```

This writes `gklib-<ver>-r0.tar.gz` (run time) and `gklib-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gklib
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gklib-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gklib-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gklib 5.1.1_git20230326-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/gklib-5.1.1_git20230326-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gklib 5.1.1_git20230326-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/gklib-5.1.1_git20230326-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **5.1.1-git-20230326**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gklib=<version>`), sources in `ported/gklib/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gklib`; port one with `scripts/port-update gklib`.

<details><summary>recipe.sh</summary>

```bash
# Pinned to the GKlib commit qnx-ports builds METIS against.
# GKlib: the support library METIS is built on (same authors). No releases.
VERSION=5.1.1-git-20230326; GIT=https://github.com/KarypisLab/GKlib.git; COMMIT=8bd6bad750b2b0d90800c632cf18e8ee93ad72d7
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DOPENMP=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
