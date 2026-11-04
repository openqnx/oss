# xtensor 0.27.1

> Header-only C++ multi-dimensional arrays with NumPy-style broadcasting

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/xtensor-stack/xtensor.git (tag `0.27.1`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`xtl`](../xtl/) [`xsimd`](../xsimd/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xtensor`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `xtensor` (programs, shared libraries, run-time data) and `xtensor-dev` (headers, pkg-config/CMake files, static libraries; needs `xtensor`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`xtensor`](https://openqnx.github.io/oss/package/x86_64/xtensor.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/xtensor.html)) · [`xtensor-dev`](https://openqnx.github.io/oss/package/x86_64/xtensor-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/xtensor-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add xtensor` (dependencies come along); one version: `apk add xtensor=<version>` | `apk del xtensor` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install xtensor` (dependencies first) | `oss-get remove xtensor` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`xtensor-dev` the same way, by its own name (`apk add xtensor-dev`, `oss-get install xtensor-dev`, by hand with its own file and list `xtensor-dev.list`); remove it before `xtensor`, which it needs.

```sh
# on the QNX target, as root
apk add xtensor xtensor-dev
apk add qnx-test-xtensor && /usr/local/bin/qnx-port-tests/test_xtensor    # its on-target test, from tests-8.0-<arch>-m-z
apk del xtensor xtensor-dev qnx-test-xtensor
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/xtensor.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/xtensor.list
rm var/lib/manual/xtensor.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xtensor` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (2 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild xtl xsimd
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b 0.27.1 --recurse-submodules --shallow-submodules \
    https://github.com/xtensor-stack/xtensor.git src/xtensor
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xtensor < ports/xtensor/patches/0001-require-xsimd-14.patch
```

**[`0001-require-xsimd-14.patch`](patches/0001-require-xsimd-14.patch)** — Require xsimd 14, as upstream does after 0.27.1.

> xsimd's CMake package accepts only requests for its own major version, so
> xtensor 0.27.1's find_package(xsimd 13.2.0) rejects xsimd 14 (the version
> ported here and the one QNX ships). Upstream xtensor moved to xsimd 14
> right after 0.27.1 with exactly this change (37e4b58, "Upgraded to xsimd 14").

Touches: `CMakeLists.txt` · Upstream status: Backport [https://github.com/xtensor-stack/xtensor/commit/37e4b5813d6df4bbbc8112be1c3fc6eff965792e]

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/xtensor -B build/$QNX_ARCH/xtensor -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DXTENSOR_USE_XSIMD=ON \
      -DXTENSOR_USE_TBB=OFF \
      -DXTENSOR_USE_OPENMP=OFF \
      -DBUILD_TESTS=OFF \
      -DBUILD_BENCHMARK=OFF
cmake --build build/$QNX_ARCH/xtensor -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/xtensor
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xtensor: header-only C++ library for multi-dimensional arrays with NumPy-style broadcasting and lazy expression evaluation. Header-only: installs include/xtensor, xtensor.pc and the CMake package.
- xtl is required; xsimd turns on SIMD evaluation of expressions (XTENSOR_USE_XSIMD): the installed xtensorConfig.cmake then finds xsimd and defines XTENSOR_USE_XSIMD for consumers, as distribution builds do.
- Header-only: name the headers a consumer needs with it.
- TBB and OpenMP parallelisation stay off (the upstream default). Tests and benchmarks are not installed: off.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xtensor`.

### 5. What it installs

81 files under `/usr/local`, 75 headers.

The complete list is `stage/<arch>/.manifest/xtensor` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xtensor
ls dists/8.0/*/ | grep '^xtensor-'
```

This writes `xtensor-<ver>-r0.tar.gz` (run time) and `xtensor-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xtensor
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xtensor-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xtensor-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xtensor`](../../tests/src/) links xtensor the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xtensor`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xtensor
/usr/local/bin/qnx-port-tests/test_xtensor
```

```
== xtensor ==
  ok   ...
PASS xtensor
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xtensor` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xtensor`](../../tests/src/) -- a program linked against xtensor the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - built with XTENSOR_USE_XSIMD
   - broadcast 3x1 * 4 -> 3x4
   - m(2,3) = 120, m(0,0) = 10
   - sum over axis 0 = [60 120 180 240]
   - sum over axis 1 = [100 200 300]
   - total 600, mean 50
   - view rows 1:3, cols ::2
   - arange(12).reshape(3,4).T
   - linspace(0,1,5)
   - lazy sqrt(a*a) + 2b evaluated on assignment
   - noalias exp(0) = 1 everywhere
   - sort
   - argmax 0, argmin 1
   - cumsum
   - adapt writes through to the std::vector
   - masked assignment of negatives
   - flip
   - npy file written and read back equal
   - printing {{1, 2},\\n {3, 4}}

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> xtensor`.

## Versions

This recipe builds **0.27.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xtensor=<version>`), sources in `ported/xtensor/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xtensor`; port one with `scripts/port-update xtensor`.

<details><summary>recipe.sh</summary>

```bash
# xtensor: header-only C++ library for multi-dimensional arrays with
# NumPy-style broadcasting and lazy expression evaluation. Header-only:
# installs include/xtensor, xtensor.pc and the CMake package.
VERSION=0.27.1; GIT=https://github.com/xtensor-stack/xtensor.git; REF=0.27.1
# xtl is required; xsimd turns on SIMD evaluation of expressions
# (XTENSOR_USE_XSIMD): the installed xtensorConfig.cmake then finds xsimd
# and defines XTENSOR_USE_XSIMD for consumers, as distribution builds do.
DEPS="xtl xsimd"
# Header-only: name the headers a consumer needs with it.
RUNTIME_DEPS="xtl-dev xsimd-dev"
BUILDSYS=cmake
# TBB and OpenMP parallelisation stay off (the upstream default). Tests and
# benchmarks are not installed: off.
CMAKE_ARGS_EXTRA=( -DXTENSOR_USE_XSIMD=ON -DXTENSOR_USE_TBB=OFF -DXTENSOR_USE_OPENMP=OFF
                   -DBUILD_TESTS=OFF -DBUILD_BENCHMARK=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
