# glslang 1.4.321.0

> Khronos reference front end for GLSL, ESSL and HLSL, and SPIR-V generator

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/KhronosGroup/glslang.git (tag `vulkan-sdk-1.4.321.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`spirv-tools`](../spirv-tools/) [`spirv-headers`](../spirv-headers/) |
| Run-time dependencies | [`spirv-tools`](../spirv-tools/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_glslang`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `glslang` (programs, shared libraries, run-time data) and `glslang-dev` (headers, pkg-config/CMake files, static libraries; needs `glslang`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`glslang`](https://openqnx.github.io/oss/package/x86_64/glslang.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/glslang.html)) · [`glslang-dev`](https://openqnx.github.io/oss/package/x86_64/glslang-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/glslang-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add glslang` (dependencies come along); one version: `apk add glslang=<version>` | `apk del glslang` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install glslang` (dependencies first) | `oss-get remove glslang` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`glslang-dev` the same way, by its own name (`apk add glslang-dev`, `oss-get install glslang-dev`, by hand with its own file and list `glslang-dev.list`); remove it before `glslang`, which it needs.

```sh
# on the QNX target, as root
apk add glslang glslang-dev
apk add qnx-test-glslang && /usr/local/bin/qnx-port-tests/test_glslang    # its on-target test, from tests-8.0-<arch>
apk del glslang glslang-dev qnx-test-glslang
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/glslang.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/glslang.list
rm var/lib/manual/glslang.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild glslang` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild spirv-tools spirv-headers
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b vulkan-sdk-1.4.321.0 --recurse-submodules --shallow-submodules \
    https://github.com/KhronosGroup/glslang.git src/glslang
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/glslang -B build/$QNX_ARCH/glslang -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DALLOW_EXTERNAL_SPIRV_TOOLS=ON \
      -DBUILD_EXTERNAL=OFF \
      -DENABLE_OPT=ON \
      -DENABLE_HLSL=ON \
      -DENABLE_GLSLANG_BINARIES=ON \
      -DGLSLANG_TESTS=OFF \
      -DBUILD_TESTING=OFF \
      -DENABLE_PCH=OFF \
      -DPython3_EXECUTABLE=/usr/bin/python3
cmake --build build/$QNX_ARCH/glslang -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/glslang
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- glslang: the Khronos reference GLSL/ESSL (and HLSL) front end and SPIR-V generator -- glslangValidator (glslang) and spirv-remap, and the libraries shaderc and Vulkan applications compile shaders with.
- - the staged SPIRV-Tools (the same Vulkan SDK release) for -O/-Os, instead of a copy checked out into External/; - shared libraries, HLSL input on, no tests; Python runs the build-info generator on the build machine.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH glslang`.

### 5. What it installs

34 files under `/usr/local`: 12 shared-library entries, 3 programs, 15 headers.

```
/usr/local/lib/libSPIRV.so
/usr/local/lib/libSPIRV.so.15
/usr/local/lib/libSPIRV.so.15.4.0
/usr/local/lib/libSPVRemapper.so
/usr/local/lib/libSPVRemapper.so.15
/usr/local/lib/libSPVRemapper.so.15.4.0
/usr/local/lib/libglslang-default-resource-limits.so
/usr/local/lib/libglslang-default-resource-limits.so.15
/usr/local/lib/libglslang-default-resource-limits.so.15.4.0
/usr/local/lib/libglslang.so
/usr/local/lib/libglslang.so.15
/usr/local/lib/libglslang.so.15.4.0
/usr/local/bin/glslang
/usr/local/bin/glslangValidator
/usr/local/bin/spirv-remap
```

The complete list is `stage/<arch>/.manifest/glslang` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa glslang
ls dists/8.0/*/ | grep '^glslang-'
```

This writes `glslang-<ver>-r0.tar.gz` (run time) and `glslang-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add glslang
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/glslang-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/glslang-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_glslang`](../../tests/src/) links glslang the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-glslang`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-glslang
/usr/local/bin/qnx-port-tests/test_glslang
```

```
== glslang ==
  ok   ...
PASS glslang
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_glslang` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_glslang`](../../tests/src/) -- a program linked against glslang the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> glslang`.

## Versions

This recipe builds **1.4.321.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add glslang=<version>`), sources in `ported/glslang/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates glslang`; port one with `scripts/port-update glslang`.

<details><summary>recipe.sh</summary>

```bash
# glslang: the Khronos reference GLSL/ESSL (and HLSL) front end and SPIR-V
# generator -- glslangValidator (glslang) and spirv-remap, and the libraries
# shaderc and Vulkan applications compile shaders with.
VERSION=1.4.321.0; GIT=https://github.com/KhronosGroup/glslang.git; REF=vulkan-sdk-1.4.321.0
DEPS="spirv-tools spirv-headers"
BUILDSYS=cmake
# - the staged SPIRV-Tools (the same Vulkan SDK release) for -O/-Os, instead
#   of a copy checked out into External/;
# - shared libraries, HLSL input on, no tests; Python runs the build-info
#   generator on the build machine.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DALLOW_EXTERNAL_SPIRV_TOOLS=ON -DBUILD_EXTERNAL=OFF
                   -DENABLE_OPT=ON -DENABLE_HLSL=ON -DENABLE_GLSLANG_BINARIES=ON
                   -DGLSLANG_TESTS=OFF -DBUILD_TESTING=OFF -DENABLE_PCH=OFF
                   -DPython3_EXECUTABLE=/usr/bin/python3 )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
