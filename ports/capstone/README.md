# capstone 5.0.9

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/capstone-engine/capstone.git (tag `5.0.9`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_capstone`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add capstone
apk add capstone-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-capstone && /usr/local/bin/qnx-port-tests/test_capstone    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install capstone`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild capstone` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 5.0.9 --recurse-submodules --shallow-submodules \
    https://github.com/capstone-engine/capstone.git src/capstone
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/capstone -B build/$QNX_ARCH/capstone -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCAPSTONE_BUILD_TESTS=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCAPSTONE_BUILD_CSTOOL=ON
cmake --build build/$QNX_ARCH/capstone -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/capstone
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- capstone: multi-architecture disassembly framework (x86, Arm, AArch64, RISC-V, ...).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH capstone`.

### 5. What it installs

30 files under `/usr/local`: 3 shared-library entries, 1 programs, 20 headers.

```
/usr/local/lib/libcapstone.so
/usr/local/lib/libcapstone.so.5
/usr/local/lib/libcapstone.so.5.0.9
/usr/local/bin/cstool
```

The complete list is `stage/<arch>/.manifest/capstone` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa capstone
ls dists/8.0/*/ | grep '^capstone-'
```

This writes `capstone-<ver>-r0.tar.gz` (run time) and `capstone-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add capstone
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/capstone-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/capstone-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_capstone`](../../tests/src/) links capstone the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-capstone`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-capstone
/usr/local/bin/qnx-port-tests/test_capstone
```

```
== capstone ==
  ok   ...
PASS capstone
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_capstone` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_capstone`](../../tests/src/) -- a program linked against capstone the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - cs_open x86-64
   - 4 instructions
   - push rbp
   - mov rbp, rsp
   - xor eax, eax
   - ret at 0x1006
   - cs_open AArch64
   - 3 instructions
   - stp x29, x30, [sp, #-0x10]!
   - mov w0, #0
   - ret

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | capstone 5.0.9-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | capstone 5.0.9-r1 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **5.0.9**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add capstone=<version>`), sources in `ported/capstone/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates capstone`; port one with `scripts/port-update capstone`.

<details><summary>recipe.sh</summary>

```bash
# capstone: multi-architecture disassembly framework (x86, Arm, AArch64, RISC-V, ...).
VERSION=5.0.9; GIT=https://github.com/capstone-engine/capstone.git; REF=5.0.9
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DCAPSTONE_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=ON -DCAPSTONE_BUILD_CSTOOL=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
