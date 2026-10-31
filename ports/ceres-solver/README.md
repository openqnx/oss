# ceres-solver 2.2.0

> C++ library for modeling and solving large optimization problems

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/ceres-solver/ceres-solver.git (tag `2.2.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`eigen`](../eigen/) [`glog`](../glog/) [`gflags`](../gflags/) |
| Run-time dependencies | [`gflags`](../gflags/) [`glog`](../glog/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_ceres_solver`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add ceres-solver
apk add ceres-solver-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-ceres-solver && /usr/local/bin/qnx-port-tests/test_ceres_solver    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install ceres-solver`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ceres-solver` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild eigen glog gflags
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b 2.2.0 --recurse-submodules --shallow-submodules \
    https://github.com/ceres-solver/ceres-solver.git src/ceres-solver
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/ceres-solver -B build/$QNX_ARCH/ceres-solver -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TESTING=OFF \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_BENCHMARKS=OFF \
      -DMINIGLOG=OFF \
      -DGFLAGS=ON \
      -DSUITESPARSE=OFF \
      -DLAPACK=OFF \
      -DCUDA=OFF \
      -DUSE_CUDA=OFF \
      -DEigen3_DIR="$STAGE$PREFIX/share/eigen3/cmake" \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/ceres-solver -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/ceres-solver
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ceres-solver`.

### 5. What it installs

71 files under `/usr/local`: 3 shared-library entries, 64 headers.

```
/usr/local/lib/libceres.so
/usr/local/lib/libceres.so.2.2.0
/usr/local/lib/libceres.so.4
```

The complete list is `stage/<arch>/.manifest/ceres-solver` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ceres-solver
ls dists/8.0/*/ | grep '^ceres-solver-'
```

This writes `ceres-solver-<ver>-r0.tar.gz` (run time) and `ceres-solver-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ceres-solver
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ceres-solver-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ceres-solver-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_ceres_solver`](../../tests/src/) links ceres-solver the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-ceres-solver`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-ceres-solver
/usr/local/bin/qnx-port-tests/test_ceres_solver
```

```
== ceres-solver ==
  ok   ...
PASS ceres-solver
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_ceres_solver` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_ceres_solver`](../../tests/src/) -- a program linked against ceres-solver the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - curve fit: solution usable
   - curve fit recovers m = 0.3, c = 0.1
   - curve fit residual ~ 0
   - Rosenbrock minimum at (1, 1) via L-BFGS
   - Cauchy loss ignores outliers: y = 2x - 5
   - central-difference Jacobian matches the analytic one

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ceres-solver 2.2.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/ceres-solver-2.2.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ceres-solver 2.2.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/ceres-solver-2.2.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ceres-solver 2.2.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/ceres-solver-2.2.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ceres-solver 2.2.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/ceres-solver-2.2.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.2.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ceres-solver=<version>`), sources in `ported/ceres-solver/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ceres-solver`; port one with `scripts/port-update ceres-solver`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.2.0; GIT=https://github.com/ceres-solver/ceres-solver.git; REF=2.2.0
DEPS="eigen glog gflags"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF
                   -DBUILD_EXAMPLES=OFF -DBUILD_BENCHMARKS=OFF
                   -DMINIGLOG=OFF -DGFLAGS=ON -DSUITESPARSE=OFF
                   -DLAPACK=OFF -DCUDA=OFF -DUSE_CUDA=OFF
                   -DEigen3_DIR="$STAGE$PREFIX/share/eigen3/cmake"
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
