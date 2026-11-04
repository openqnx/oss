# spirv-tools 1.4.321.0

> SPIR-V assembler, disassembler, validator, optimizer and linker

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/KhronosGroup/SPIRV-Tools.git (tag `vulkan-sdk-1.4.321.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`spirv-headers`](../spirv-headers/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_spirv_tools`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `spirv-tools` (programs, shared libraries, run-time data) and `spirv-tools-dev` (headers, pkg-config/CMake files, static libraries; needs `spirv-tools`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`spirv-tools`](https://openqnx.github.io/oss/package/x86_64/spirv-tools.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/spirv-tools.html)) · [`spirv-tools-dev`](https://openqnx.github.io/oss/package/x86_64/spirv-tools-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/spirv-tools-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add spirv-tools` (dependencies come along); one version: `apk add spirv-tools=<version>` | `apk del spirv-tools` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install spirv-tools` (dependencies first) | `oss-get remove spirv-tools` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`spirv-tools-dev` the same way, by its own name (`apk add spirv-tools-dev`, `oss-get install spirv-tools-dev`, by hand with its own file and list `spirv-tools-dev.list`); remove it before `spirv-tools`, which it needs.

```sh
# on the QNX target, as root
apk add spirv-tools spirv-tools-dev
apk add qnx-test-spirv-tools && /usr/local/bin/qnx-port-tests/test_spirv_tools    # its on-target test, from tests-8.0-<arch>-m-z
apk del spirv-tools spirv-tools-dev qnx-test-spirv-tools
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/spirv-tools.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/spirv-tools.list
rm var/lib/manual/spirv-tools.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild spirv-tools` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild spirv-headers
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b vulkan-sdk-1.4.321.0 --recurse-submodules --shallow-submodules \
    https://github.com/KhronosGroup/SPIRV-Tools.git src/spirv-tools
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/spirv-tools -B build/$QNX_ARCH/spirv-tools -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DSPIRV-Headers_SOURCE_DIR="$STAGE$PREFIX" \
      -DPython3_EXECUTABLE=/usr/bin/python3 \
      -DBUILD_SHARED_LIBS=ON \
      -DSPIRV_TOOLS_BUILD_STATIC=OFF \
      -DSPIRV_SKIP_TESTS=ON \
      -DSPIRV_WERROR=OFF \
      -DBUILD_TESTING=OFF
cmake --build build/$QNX_ARCH/spirv-tools -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/spirv-tools
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- spirv-tools: the SPIR-V assembler, disassembler, validator, optimizer and linker (spirv-as, spirv-dis, spirv-val, spirv-opt, spirv-link ...) and their library. Pinned to the same Vulkan SDK release as spirv-headers and glslang.
- - the staged spirv-headers instead of a copy in external/ (the generated tables come from its grammar files); - the generators are Python scripts run on the build machine; - shared libraries only (as distributions ship it: glslang and shaderc link libSPIRV-Tools-opt.so), no tests, no -Werror (GCC 12 warnings).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH spirv-tools`.

### 5. What it installs

44 files under `/usr/local`: 7 shared-library entries, 10 programs, 4 headers.

```
/usr/local/lib/libSPIRV-Tools-diff.so
/usr/local/lib/libSPIRV-Tools-link.so
/usr/local/lib/libSPIRV-Tools-lint.so
/usr/local/lib/libSPIRV-Tools-opt.so
/usr/local/lib/libSPIRV-Tools-reduce.so
/usr/local/lib/libSPIRV-Tools-shared.so
/usr/local/lib/libSPIRV-Tools.so
/usr/local/bin/spirv-as
/usr/local/bin/spirv-cfg
/usr/local/bin/spirv-dis
/usr/local/bin/spirv-lesspipe.sh
/usr/local/bin/spirv-link
/usr/local/bin/spirv-lint
/usr/local/bin/spirv-objdump
/usr/local/bin/spirv-opt
/usr/local/bin/spirv-reduce
...
```

The complete list is `stage/<arch>/.manifest/spirv-tools` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa spirv-tools
ls dists/8.0/*/ | grep '^spirv-tools-'
```

This writes `spirv-tools-<ver>-r0.tar.gz` (run time) and `spirv-tools-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add spirv-tools
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/spirv-tools-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/spirv-tools-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_spirv_tools`](../../tests/src/) links spirv-tools the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-spirv-tools`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-spirv-tools
/usr/local/bin/qnx-port-tests/test_spirv_tools
```

```
== spirv-tools ==
  ok   ...
PASS spirv-tools
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_spirv_tools` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_spirv_tools`](../../tests/src/) -- a program linked against spirv-tools the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> spirv-tools`.

## Versions

This recipe builds **1.4.321.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add spirv-tools=<version>`), sources in `ported/spirv-tools/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates spirv-tools`; port one with `scripts/port-update spirv-tools`.

<details><summary>recipe.sh</summary>

```bash
# spirv-tools: the SPIR-V assembler, disassembler, validator, optimizer and
# linker (spirv-as, spirv-dis, spirv-val, spirv-opt, spirv-link ...) and their
# library. Pinned to the same Vulkan SDK release as spirv-headers and glslang.
VERSION=1.4.321.0; GIT=https://github.com/KhronosGroup/SPIRV-Tools.git; REF=vulkan-sdk-1.4.321.0
DEPS="spirv-headers"
BUILDSYS=cmake
# - the staged spirv-headers instead of a copy in external/ (the generated
#   tables come from its grammar files);
# - the generators are Python scripts run on the build machine;
# - shared libraries only (as distributions ship it: glslang and shaderc link
#   libSPIRV-Tools-opt.so), no tests, no -Werror (GCC 12 warnings).
CMAKE_ARGS_EXTRA=( -DSPIRV-Headers_SOURCE_DIR="$STAGE$PREFIX"
                   -DPython3_EXECUTABLE=/usr/bin/python3
                   -DBUILD_SHARED_LIBS=ON -DSPIRV_TOOLS_BUILD_STATIC=OFF
                   -DSPIRV_SKIP_TESTS=ON -DSPIRV_WERROR=OFF -DBUILD_TESTING=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
