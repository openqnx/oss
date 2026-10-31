# metis 5.2.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/KarypisLab/METIS.git (tag `v5.2.1`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`gklib`](../gklib/) |
| Run-time dependencies | [`gklib`](../gklib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add metis
apk add metis-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install metis`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild metis` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild gklib
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v5.2.1 --recurse-submodules --shallow-submodules \
    https://github.com/KarypisLab/METIS.git src/metis
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/metis < ports/metis/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Port METIS minus tests.

> Imported from qnx-ports/METIS (f5ae915a84d3...qnx_v5.2.1); original authorship kept.
>
>
> ---
>  conf/gkbuild.cmake      | 2 +-
>  include/CMakeLists.txt  | 6 +++++-
>  programs/CMakeLists.txt | 2 +-
>  test/CMakeLists.txt     | 8 ++++++++
>  4 files changed, 15 insertions(+), 3 deletions(-)
>  create mode 100644 test/CMakeLists.txt

Touches: `conf/gkbuild.cmake`, `include/CMakeLists.txt`, `programs/CMakeLists.txt`, `test/CMakeLists.txt`, `programs/CMakeLists.txt` · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/metis -B build/$QNX_ARCH/metis -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCMAKE_INSTALL_INCLUDEDIR=include \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DCMAKE_INSTALL_BINDIR=bin \
      -DGKLIB_PATH="$STAGE$PREFIX" \
      -DSHARED=ON
cmake --build build/$QNX_ARCH/metis -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/metis
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- METIS's CMake build expects "make config" to have run first: it writes build/xinclude/metis.h with the index and real widths (32-bit here, as upstream's default) and copies the header's CMakeLists next to it.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH metis`.

### 5. What it installs

8 files under `/usr/local`: 1 shared-library entries, 6 programs, 1 headers.

```
/usr/local/lib/libmetis.so
/usr/local/bin/cmpfillin
/usr/local/bin/gpmetis
/usr/local/bin/graphchk
/usr/local/bin/m2gmetis
/usr/local/bin/mpmetis
/usr/local/bin/ndmetis
```

The complete list is `stage/<arch>/.manifest/metis` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa metis
ls dists/8.0/*/ | grep '^metis-'
```

This writes `metis-<ver>-r0.tar.gz` (run time) and `metis-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add metis
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/metis-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/metis-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | metis 5.2.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/metis-5.2.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | metis 5.2.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/metis-5.2.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **5.2.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add metis=<version>`), sources in `ported/metis/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates metis`; port one with `scripts/port-update metis`.

<details><summary>recipe.sh</summary>

```bash
VERSION=5.2.1; GIT=https://github.com/KarypisLab/METIS.git; REF=v5.2.1
DEPS="gklib"
CMAKE_ARGS_EXTRA=( -DCMAKE_INSTALL_INCLUDEDIR=include -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_BINDIR=bin -DGKLIB_PATH="$STAGE$PREFIX" -DSHARED=ON )
# METIS's CMake build expects "make config" to have run first: it writes
# build/xinclude/metis.h with the index and real widths (32-bit here, as
# upstream's default) and copies the header's CMakeLists next to it.
do_patch() {
  cd "$SRCROOT" && mkdir -p build/xinclude && \
  { echo "#define IDXTYPEWIDTH 32"; echo "#define REALTYPEWIDTH 32"; cat include/metis.h; } > build/xinclude/metis.h && \
  cp include/CMakeLists.txt build/xinclude/
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
