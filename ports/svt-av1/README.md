# svt-av1 2.3.0

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

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install svt-av1
oss-get install svt-av1-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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
./scripts/qnxpkg -a $QNX_ARCH svt-av1
ls repo/8.0/*/ | grep '^svt-av1-'
```

This writes `svt-av1-<ver>-r0.tar.gz` (run time) and `svt-av1-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install svt-av1
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/svt-av1-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/svt-av1-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_svt_av1`](../../tests/src/) links svt-av1 the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_svt_av1 root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_svt_av1'
```

```
== svt-av1 ==
  ok   ...
PASS svt-av1
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

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
