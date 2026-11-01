# mnn 3.6.2

> Lightweight deep learning inference engine from Alibaba

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/alibaba/MNN.git (tag `3.6.2`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_mnn`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `mnn` (programs, shared libraries, run-time data) and `mnn-dev` (headers, pkg-config/CMake files, static libraries; needs `mnn`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`mnn`](https://openqnx.github.io/oss/package/x86_64/mnn.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/mnn.html)) · [`mnn-dev`](https://openqnx.github.io/oss/package/x86_64/mnn-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/mnn-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add mnn` (dependencies come along); one version: `apk add mnn=<version>` | `apk del mnn` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install mnn` (dependencies first) | `oss-get remove mnn` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`mnn-dev` the same way, by its own name (`apk add mnn-dev`, `oss-get install mnn-dev`, by hand with its own file and list `mnn-dev.list`); remove it before `mnn`, which it needs.

```sh
# on the QNX target, as root
apk add mnn mnn-dev
apk add qnx-test-mnn && /usr/local/bin/qnx-port-tests/test_mnn    # its on-target test, from tests-8.0-<arch>-m-z
apk del mnn mnn-dev qnx-test-mnn
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/mnn.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/mnn.list
rm var/lib/manual/mnn.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild mnn` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 3.6.2 --recurse-submodules --shallow-submodules \
    https://github.com/alibaba/MNN.git src/mnn
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/mnn -B build/$QNX_ARCH/mnn -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DMNN_BUILD_SHARED_LIBS=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DMNN_SEP_BUILD=ON \
      -DMNN_USE_THREAD_POOL=ON \
      -DMNN_OPENMP=OFF \
      -DMNN_VULKAN=OFF \
      -DMNN_OPENCL=OFF \
      -DMNN_OPENGL=OFF \
      -DMNN_BUILD_CONVERTER=OFF \
      -DMNN_BUILD_TOOLS=OFF \
      -DMNN_BUILD_DEMO=OFF \
      -DMNN_BUILD_TEST=OFF \
      -DMNN_BUILD_BENCHMARK=OFF
cmake --build build/$QNX_ARCH/mnn -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/mnn
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- MNN: Alibaba's lightweight deep-learning inference engine (CPU), as the shared libraries libMNN and libMNN_Express (the express API that builds and runs graphs in code, and the Module API). QNX's changes (github.com/qnx-ports/MNN, "add compatibility support for QNX Neutrino") are upstream since 3.1.2.
- Threads through MNN's own pool: QNX's GCC has no OpenMP runtime. No GPU backends (no Vulkan or OpenCL in the SDP), no model converter (it would need its own protobuf), no tools (MNN does not install them). -fPIC: the library is linked from CMake object libraries, which on x86_64 are not PIC by themselves.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH mnn`.

### 5. What it installs

22 files under `/usr/local`: 2 shared-library entries, 20 headers.

```
/usr/local/lib/libMNN.so
/usr/local/lib/libMNN_Express.so
```

The complete list is `stage/<arch>/.manifest/mnn` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa mnn
ls dists/8.0/*/ | grep '^mnn-'
```

This writes `mnn-<ver>-r0.tar.gz` (run time) and `mnn-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add mnn
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/mnn-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/mnn-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_mnn`](../../tests/src/) links mnn the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-mnn`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-mnn
/usr/local/bin/qnx-port-tests/test_mnn
```

```
== mnn ==
  ok   ...
PASS mnn
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_mnn` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_mnn`](../../tests/src/) -- a program linked against mnn the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - graph computed: 4 outputs
   - conv + relu + sum + matmul + softmax match the reference
   - softmax sums to 1
   - graph serialized to an MNN model
   - model loaded as a Module
   - Module::onForward on a second input matches the reference
   - model loaded by the Interpreter
   - session ran (2 threads)
   - Interpreter session output matches the reference

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | mnn 3.6.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/mnn-3.6.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | mnn 3.6.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/mnn-3.6.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.6.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add mnn=<version>`), sources in `ported/mnn/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates mnn`; port one with `scripts/port-update mnn`.

<details><summary>recipe.sh</summary>

```bash
# MNN: Alibaba's lightweight deep-learning inference engine (CPU), as the
# shared libraries libMNN and libMNN_Express (the express API that builds and
# runs graphs in code, and the Module API). QNX's changes
# (github.com/qnx-ports/MNN, "add compatibility support for QNX Neutrino") are
# upstream since 3.1.2.
VERSION=3.6.2; GIT=https://github.com/alibaba/MNN.git; REF=3.6.2
DEPS=""
BUILDSYS=cmake
# Threads through MNN's own pool: QNX's GCC has no OpenMP runtime. No GPU
# backends (no Vulkan or OpenCL in the SDP), no model converter (it would
# need its own protobuf), no tools (MNN does not install them). -fPIC: the
# library is linked from CMake object libraries, which on x86_64 are not PIC
# by themselves.
CMAKE_ARGS_EXTRA=( -DMNN_BUILD_SHARED_LIBS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DMNN_SEP_BUILD=ON -DMNN_USE_THREAD_POOL=ON -DMNN_OPENMP=OFF
                   -DMNN_VULKAN=OFF -DMNN_OPENCL=OFF -DMNN_OPENGL=OFF -DMNN_BUILD_CONVERTER=OFF
                   -DMNN_BUILD_TOOLS=OFF -DMNN_BUILD_DEMO=OFF -DMNN_BUILD_TEST=OFF -DMNN_BUILD_BENCHMARK=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
