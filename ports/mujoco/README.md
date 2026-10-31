# mujoco 3.10.0

> MuJoCo physics engine for multi-joint dynamics with contact

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/google-deepmind/mujoco.git (tag `3.10.0`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_mujoco`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add mujoco
apk add mujoco-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-mujoco && /usr/local/bin/qnx-port-tests/test_mujoco    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install mujoco`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild mujoco` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 3.10.0 --recurse-submodules --shallow-submodules \
    https://github.com/google-deepmind/mujoco.git src/mujoco
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/mujoco < ports/mujoco/patches/0001-qnx-localtime-r.patch
patch -p1 -d src/mujoco < ports/mujoco/patches/0002-qnx-sys-link-h.patch
```

**[`0001-qnx-localtime-r.patch`](patches/0001-qnx-localtime-r.patch)** — Use localtime_r on QNX

> mju_localTimeStr() picks localtime_r() when _POSIX_C_SOURCE is defined, and
> stops with #error otherwise. QNX's C library has localtime_r(), but its
> headers do not define _POSIX_C_SOURCE unless the program asks for it.

Touches: `src/engine/engine_util_errmem.c` · Upstream status: Pending

**[`0002-qnx-sys-link-h.patch`](patches/0002-qnx-sys-link-h.patch)** — Take dl_iterate_phdr from <sys/link.h> on QNX

> The OpenGL loader finds which GL platform library is loaded with
> dl_iterate_phdr(), declared in <link.h> on Linux and the BSDs. QNX declares
> it, with struct dl_phdr_info, in <sys/link.h> and has no <link.h>.

Touches: `src/render/classic/glad/glad.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/mujoco -B build/$QNX_ARCH/mujoco -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCMAKE_C_FLAGS=-Dinline=__inline__ \
      -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF \
      -DMUJOCO_BUILD_EXAMPLES=OFF \
      -DMUJOCO_BUILD_SIMULATE=OFF \
      -DMUJOCO_BUILD_TESTS=OFF \
      -DMUJOCO_TEST_PYTHON_UTIL=OFF \
      -DMUJOCO_ENABLE_AVX=OFF \
      -DMUJOCO_ENABLE_AVX_INTRINSICS=OFF
cmake --build build/$QNX_ARCH/mujoco -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/mujoco
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- mujoco: DeepMind's MuJoCo physics engine (multi-body dynamics with contact), the library and its model compiler; no simulate viewer (needs GLFW/OpenGL). Its dependencies (tinyxml2, qhull, ccd, lodepng, ...) are fetched at configure time at the versions MuJoCo pins, and linked in statically.
- Link-time optimisation is on by default for release builds and CMake has no IPO support for QNX's compiler ("CMake doesn't support IPO"): off. libccd (fetched) compiles as C90, and QNX's C headers use "static inline": for C, inline is spelled __inline__, which GCC takes in every mode and gives the mode's own inline semantics.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH mujoco`.

### 5. What it installs

199 files under `/usr/local`: 2 shared-library entries, 16 headers.

```
/usr/local/lib/libmujoco.so
/usr/local/lib/libmujoco.so.3.10.0
```

The complete list is `stage/<arch>/.manifest/mujoco` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa mujoco
ls dists/8.0/*/ | grep '^mujoco-'
```

This writes `mujoco-<ver>-r0.tar.gz` (run time) and `mujoco-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add mujoco
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/mujoco-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/mujoco-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_mujoco`](../../tests/src/) links mujoco the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-mujoco`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-mujoco
/usr/local/bin/qnx-port-tests/test_mujoco
```

```
== mujoco ==
  ok   ...
PASS mujoco
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_mujoco` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_mujoco`](../../tests/src/) -- a program linked against mujoco the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - mj_loadXML (MJCF compiled)
   - 2 bodies, 2 geoms, free joint (nq 7)
   - free fall at 0.3 s matches 1 - g t^2 / 2
   - at rest on the plane (z = radius)

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | mujoco 3.10.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/mujoco-3.10.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | mujoco 3.10.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/mujoco-3.10.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.10.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add mujoco=<version>`), sources in `ported/mujoco/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates mujoco`; port one with `scripts/port-update mujoco`.

<details><summary>recipe.sh</summary>

```bash
# mujoco: DeepMind's MuJoCo physics engine (multi-body dynamics with contact),
# the library and its model compiler; no simulate viewer (needs GLFW/OpenGL).
# Its dependencies (tinyxml2, qhull, ccd, lodepng, ...) are fetched at configure
# time at the versions MuJoCo pins, and linked in statically.
VERSION=3.10.0; GIT=https://github.com/google-deepmind/mujoco.git; REF=3.10.0
DEPS=""
BUILDSYS=cmake
# Link-time optimisation is on by default for release builds and CMake has no
# IPO support for QNX's compiler ("CMake doesn't support IPO"): off.
# libccd (fetched) compiles as C90, and QNX's C headers use "static inline":
# for C, inline is spelled __inline__, which GCC takes in every mode and
# gives the mode's own inline semantics.
CMAKE_ARGS_EXTRA=( -DCMAKE_C_FLAGS=-Dinline=__inline__ -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF -DMUJOCO_BUILD_EXAMPLES=OFF -DMUJOCO_BUILD_SIMULATE=OFF -DMUJOCO_BUILD_TESTS=OFF -DMUJOCO_TEST_PYTHON_UTIL=OFF -DMUJOCO_ENABLE_AVX=OFF -DMUJOCO_ENABLE_AVX_INTRINSICS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
