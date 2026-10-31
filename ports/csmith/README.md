# csmith 2.3.0

> Random generator of C programs for testing compilers

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/csmith-project/csmith.git (tag `csmith-2.3.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_csmith`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add csmith
apk add csmith-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-csmith && /usr/local/bin/qnx-port-tests/test_csmith    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install csmith`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild csmith` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b csmith-2.3.0 --recurse-submodules --shallow-submodules \
    https://github.com/csmith-project/csmith.git src/csmith
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/csmith -B build/$QNX_ARCH/csmith -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCMAKE_CXX_FLAGS=-std=gnu++17 \
      -isystem \
      $STAGE$PREFIX/include \
      -D_LIBCPP_ENABLE_CXX17_REMOVED_BINDERS
cmake --build build/$QNX_ARCH/csmith -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/csmith
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- csmith 2.3.0 still uses std::bind2nd/std::ptr_fun, which C++17 removed. The QNX toolchain builds as gnu++17 by default; libc++ keeps the old binders available when asked to. (CMAKE_CXX_FLAGS replaces the toolchain's initial flags, so those are repeated here.)

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH csmith`.

### 5. What it installs

28 files under `/usr/local`: 3 shared-library entries, 4 programs, 19 headers.

```
/usr/local/lib/libcsmith.so
/usr/local/lib/libcsmith.so.0
/usr/local/lib/libcsmith.so.0.0.0
/usr/local/bin/compiler_test.in
/usr/local/bin/compiler_test.pl
/usr/local/bin/csmith
/usr/local/bin/launchn.pl
```

The complete list is `stage/<arch>/.manifest/csmith` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa csmith
ls dists/8.0/*/ | grep '^csmith-'
```

This writes `csmith-<ver>-r0.tar.gz` (run time) and `csmith-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add csmith
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/csmith-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/csmith-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_csmith`](../../tests/src/) links csmith the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-csmith`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-csmith
/usr/local/bin/qnx-port-tests/test_csmith
```

```
== csmith ==
  ok   ...
PASS csmith
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_csmith` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_csmith`](../../tests/src/) -- a program linked against csmith the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | csmith 2.3.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/csmith-2.3.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | csmith 2.3.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/csmith-2.3.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | csmith 2.3.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/csmith-2.3.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | csmith 2.3.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/csmith-2.3.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.3.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add csmith=<version>`), sources in `ported/csmith/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates csmith`; port one with `scripts/port-update csmith`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.3.0; GIT=https://github.com/csmith-project/csmith.git; REF=csmith-2.3.0
# csmith 2.3.0 still uses std::bind2nd/std::ptr_fun, which C++17 removed. The
# QNX toolchain builds as gnu++17 by default; libc++ keeps the old binders
# available when asked to. (CMAKE_CXX_FLAGS replaces the toolchain's initial
# flags, so those are repeated here.)
CMAKE_ARGS_EXTRA=( "-DCMAKE_CXX_FLAGS=-std=gnu++17 -isystem $STAGE$PREFIX/include -D_LIBCPP_ENABLE_CXX17_REMOVED_BINDERS" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
