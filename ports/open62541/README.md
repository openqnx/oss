# open62541 1.4.8

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/open62541/open62541.git (tag `v1.4.8`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`openssl`](../openssl/) |
| Run-time dependencies | [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_open62541`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install open62541
oss-get install open62541-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild open62541` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild openssl
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.4.8 --recurse-submodules --shallow-submodules \
    https://github.com/open62541/open62541.git src/open62541
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/open62541 -B build/$QNX_ARCH/open62541 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DUA_ENABLE_ENCRYPTION=OPENSSL \
      -DUA_BUILD_EXAMPLES=OFF \
      -DUA_BUILD_UNIT_TESTS=OFF \
      -DUA_FORCE_WERROR=OFF \
      -DUA_ENABLE_AMALGAMATION=OFF \
      -DPython3_EXECUTABLE=/usr/bin/python3 \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/open62541 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/open62541
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The node-set and data-type code generators are Python scripts that run on the build host. The QNX toolchain file restricts find_package to the target sysroot, so name the host interpreter explicitly. The library is assembled from CMake OBJECT libraries, which do not inherit PIC from BUILD_SHARED_LIBS; on x86_64 the shared link then fails with "relocation R_X86_64_PC32 ... recompile with -fPIC".

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH open62541`.

### 5. What it installs

73 files under `/usr/local`: 3 shared-library entries, 36 headers.

```
/usr/local/lib/libopen62541.so
/usr/local/lib/libopen62541.so.1.4
/usr/local/lib/libopen62541.so.1.4.8
```

The complete list is `stage/<arch>/.manifest/open62541` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH open62541
ls repo/8.0/*/ | grep '^open62541-'
```

This writes `open62541-<ver>-r0.tar.gz` (run time) and `open62541-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install open62541
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/open62541-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/open62541-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_open62541`](../../tests/src/) links open62541 the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_open62541 root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_open62541'
```

```
== open62541 ==
  ok   ...
PASS open62541
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.4.8; GIT=https://github.com/open62541/open62541.git; REF=v1.4.8
DEPS="openssl"
# The node-set and data-type code generators are Python scripts that run on the
# build host. The QNX toolchain file restricts find_package to the target
# sysroot, so name the host interpreter explicitly.
# The library is assembled from CMake OBJECT libraries, which do not inherit
# PIC from BUILD_SHARED_LIBS; on x86_64 the shared link then fails with
# "relocation R_X86_64_PC32 ... recompile with -fPIC".
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DUA_ENABLE_ENCRYPTION=OPENSSL -DUA_BUILD_EXAMPLES=OFF
                   -DUA_BUILD_UNIT_TESTS=OFF -DUA_FORCE_WERROR=OFF -DUA_ENABLE_AMALGAMATION=OFF
                   -DPython3_EXECUTABLE=/usr/bin/python3
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
