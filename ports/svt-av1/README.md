# svt-av1 2.3.0

> Scalable Video Technology AV1 encoder library and tool

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://gitlab.com/AOMediaCodec/SVT-AV1.git (tag `v2.3.0`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_svt_av1`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add svt-av1
apk add svt-av1-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-svt-av1 && /usr/local/bin/qnx-port-tests/test_svt_av1    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install svt-av1`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild svt-av1` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2.3.0 --recurse-submodules --shallow-submodules \
    https://gitlab.com/AOMediaCodec/SVT-AV1.git src/svt-av1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/svt-av1 < ports/svt-av1/patches/0001-qnx-wl-z-linker-flags.patch
patch -p1 -d src/svt-av1 < ports/svt-av1/patches/0002-cpuinfo-qnx-x86-isa-detection.patch
```

**[`0001-qnx-wl-z-linker-flags.patch`](patches/0001-qnx-wl-z-linker-flags.patch)** — SVT-AV1: pass -z linker options through -Wl.

> The qcc driver does not forward a bare "-z foo". Every try_compile() after
> these flags failed, silently turning off __builtin_expect, threads and the
> NEON dotprod/i8mm paths.

Touches: `CMakeLists.txt` · Upstream status: Pending

**[`0002-cpuinfo-qnx-x86-isa-detection.patch`](patches/0002-cpuinfo-qnx-x86-isa-detection.patch)** — SVT-AV1's bundled cpuinfo: x86 ISA detection on QNX.

> Use the BSD initialiser, which is pure CPUID, for QNX; otherwise src/x86/isa.c
> is never compiled and libSvtAv1Enc has an unresolved cpuinfo_isa.

Touches: `third_party/cpuinfo/CMakeLists.txt`, `third_party/cpuinfo/src/init.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/svt-av1 -B build/$QNX_ARCH/svt-av1 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TESTING=OFF \
      -DBUILD_APPS=OFF \
      -DENABLE_NASM=ON
cmake --build build/$QNX_ARCH/svt-av1 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/svt-av1
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH svt-av1`.

### 5. What it installs

11 files under `/usr/local`: 3 shared-library entries, 7 headers.

```
/usr/local/lib/libSvtAv1Enc.so
/usr/local/lib/libSvtAv1Enc.so.2
/usr/local/lib/libSvtAv1Enc.so.2.3.0
```

The complete list is `stage/<arch>/.manifest/svt-av1` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa svt-av1
ls dists/8.0/*/ | grep '^svt-av1-'
```

This writes `svt-av1-<ver>-r0.tar.gz` (run time) and `svt-av1-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add svt-av1
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/svt-av1-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/svt-av1-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_svt_av1`](../../tests/src/) links svt-av1 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-svt-av1`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-svt-av1
/usr/local/bin/qnx-port-tests/test_svt_av1
```

```
== svt-av1 ==
  ok   ...
PASS svt-av1
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_svt_av1` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_svt_av1`](../../tests/src/) -- a program linked against svt-av1 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - svt_av1_enc_init_handle
   - svt_av1_enc_set_parameter
   - svt_av1_enc_init
   - svt_av1_enc_stream_header
   - sequence header bytes produced
   - frames accepted by the encoder
   - encoded packets returned

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | svt-av1 2.3.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/svt-av1-2.3.0-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | svt-av1 2.3.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/svt-av1-2.3.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.3.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add svt-av1=<version>`), sources in `ported/svt-av1/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates svt-av1`; port one with `scripts/port-update svt-av1`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.3.0; GIT=https://gitlab.com/AOMediaCodec/SVT-AV1.git; REF=v2.3.0
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DBUILD_APPS=OFF
                   -DENABLE_NASM=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
