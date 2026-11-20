# sophus 1.24.6

> C++ Lie groups (SO2, SO3, SE2, SE3, Sim3) for 2D and 3D geometry on Eigen, header-only

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/strasdat/Sophus.git (tag `1.24.6`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`eigen`](../eigen/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_sophus`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `sophus` (programs, shared libraries, run-time data) and `sophus-dev` (headers, pkg-config/CMake files, static libraries; needs `sophus`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`sophus`](https://openqnx.github.io/oss/package/x86_64/sophus.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/sophus.html)) · [`sophus-dev`](https://openqnx.github.io/oss/package/x86_64/sophus-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/sophus-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add sophus` (dependencies come along); one version: `apk add sophus=<version>` | `apk del sophus` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install sophus` (dependencies first) | `oss-get remove sophus` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`sophus-dev` the same way, by its own name (`apk add sophus-dev`, `oss-get install sophus-dev`, by hand with its own file and list `sophus-dev.list`); remove it before `sophus`, which it needs.

```sh
# on the QNX target, as root
apk add sophus sophus-dev
apk add qnx-test-sophus && /usr/local/bin/qnx-port-tests/test_sophus    # its on-target test, from tests-8.0-<arch>-m-z
apk del sophus sophus-dev qnx-test-sophus
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/sophus.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/sophus.list
rm var/lib/manual/sophus.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild sophus` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild eigen
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.24.6 --recurse-submodules --shallow-submodules \
    https://github.com/strasdat/Sophus.git src/sophus
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/sophus -B build/$QNX_ARCH/sophus -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SOPHUS_TESTS=OFF \
      -DBUILD_PYTHON_BINDINGS=OFF \
      -DSOPHUS_INSTALL=ON
cmake --build build/$QNX_ARCH/sophus -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/sophus
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- sophus: C++ Lie groups for 2D and 3D geometry on Eigen -- SO(2), SO(3), SE(2), SE(3), RxSO(2/3), Sim(2/3) with exp/log, group actions, Jacobians, interpolation and averaging. Header-only: the port installs the headers in include/sophus/ and the CMake package (Sophus::Sophus, which pulls in Eigen3::Eigen) in share/sophus/cmake. Nothing had to change for QNX. Upstream's test programs are not built (BUILD_SOPHUS_TESTS=OFF): they would need Ceres for part of them and cannot run on the build machine anyway; tests/src/test_sophus.cpp checks the group operations on the target instead. 1.24.6 formats its messages with iostreams, so there is no dependency on fmt as in the 1.22 series.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH sophus`.

### 5. What it installs

24 files under `/usr/local`, 21 headers.

The complete list is `stage/<arch>/.manifest/sophus` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa sophus
ls dists/8.0/*/ | grep '^sophus-'
```

This writes `sophus-<ver>-r0.tar.gz` (run time) and `sophus-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add sophus
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/sophus-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/sophus-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_sophus`](../../tests/src/) links sophus the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-sophus`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-sophus
/usr/local/bin/qnx-port-tests/test_sophus
```

```
== sophus ==
  ok   ...
PASS sophus
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_sophus` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_sophus`](../../tests/src/) -- a program linked against sophus the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - SO2 exp(pi/6) is the 30 degree rotation matrix
   - three 30 degree rotations have log pi/2
   - SO2 quarter turn maps x to y
   - SO3 exp(0,0,pi/2) turns x to y, y to -x, keeps z
   - its quaternion is (sqrt(1/2); 0, 0, sqrt(1/2))
   - SO3 log(exp(w)) = w
   - exp(w) exp(-w) = I
   - exp(w) is orthonormal with determinant 1
   - SO3 action equals Rodrigues' formula
   - hat/vee: skew-symmetric matrix and back
   - rotX(pi) = diag(1,-1,-1)
   - SE3 (Rz90, t=(1,2,3)) maps (1,0,0) to (1,3,3)
   - its inverse maps it back
   - T T^-1 = I
   - T T: translation (-1,3,6), rotation pi about z
   - SE3 exp of a screw twist is a quarter circle ending at (1,1,0)
   - SE3 log(exp(xi)) = xi
   - pure translation twist
   - adjoint identity T exp(a) T^-1 = exp(Adj(T) a)
   - interpolation halfway turns pi/4
   - the halfway pose applied twice gives the end pose
   - interpolation end points
   - average of rotations 0.2, 0.5, 0.8 about z is 0.5
   - SE2 (90 degrees, t=(1,0)) maps (1,0) to (1,1)
   - SE2 exp of the arc twist ends at (1,1); log inverts it
   - ... and 4 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | sophus 1.24.6-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/aarch64le/sophus-1.24.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | sophus 1.24.6-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/x86_64/sophus-1.24.6-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.24.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add sophus=<version>`), sources in `ported/sophus/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates sophus`; port one with `scripts/port-update sophus`.

<details><summary>recipe.sh</summary>

```bash
# sophus: C++ Lie groups for 2D and 3D geometry on Eigen -- SO(2), SO(3),
# SE(2), SE(3), RxSO(2/3), Sim(2/3) with exp/log, group actions, Jacobians,
# interpolation and averaging. Header-only: the port installs the headers in
# include/sophus/ and the CMake package (Sophus::Sophus, which pulls in
# Eigen3::Eigen) in share/sophus/cmake.
#
# Nothing had to change for QNX. Upstream's test programs are not built
# (BUILD_SOPHUS_TESTS=OFF): they would need Ceres for part of them and
# cannot run on the build machine anyway; tests/src/test_sophus.cpp checks
# the group operations on the target instead. 1.24.6 formats its messages
# with iostreams, so there is no dependency on fmt as in the 1.22 series.
VERSION=1.24.6; GIT=https://github.com/strasdat/Sophus.git; REF=1.24.6
DEPS="eigen"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_SOPHUS_TESTS=OFF -DBUILD_PYTHON_BINDINGS=OFF -DSOPHUS_INSTALL=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
