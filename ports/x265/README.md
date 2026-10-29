# x265 3.6

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://bitbucket.org/multicoreware/x265_git.git (tag `3.6`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_x265`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install x265
oss-get install x265-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild x265` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 3.6 --recurse-submodules --shallow-submodules \
    https://bitbucket.org/multicoreware/x265_git.git src/x265
```

The project lives in the `source/` subdirectory of the repository.

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/x265 < ports/x265/patches/0001-cmake4-policies.patch
```

**[`0001-cmake4-policies.patch`](patches/0001-cmake4-policies.patch)** — x265: set CMP0025 and CMP0054 to NEW for CMake 4.

> CMake 4 removed the OLD behaviour of these policies and refuses to configure.
> CMP0025 only affects Apple's compiler name; NEW CMP0054 is the long-standing
> default.

Touches: `source/CMakeLists.txt` · Upstream status: Pending (not QNX-specific: CMake 4)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/x265/source -B build/$QNX_ARCH/x265 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DENABLE_SHARED=ON \
      -DENABLE_CLI=OFF \
      -DENABLE_TESTS=OFF \
      -DENABLE_ASSEMBLY=OFF \
      # \
      nasm/yasm \
      absent \
      on \
      this \
      host \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/x265 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/x265
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH x265`.

### 5. What it installs

6 files under `/usr/local`: 2 shared-library entries, 2 headers.

```
/usr/local/lib/libx265.so
/usr/local/lib/libx265.so.209
```

The complete list is `stage/<arch>/.manifest/x265` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH x265
ls repo/8.0/*/ | grep '^x265-'
```

This writes `x265-<ver>-r0.tar.gz` (run time) and `x265-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install x265
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/x265-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/x265-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_x265`](../../tests/src/) links x265 the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_x265 root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_x265'
```

```
== x265 ==
  ok   ...
PASS x265
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.6; GIT=https://bitbucket.org/multicoreware/x265_git.git; REF=3.6
SUBDIR=source
CMAKE_ARGS_EXTRA=( -DENABLE_SHARED=ON -DENABLE_CLI=OFF -DENABLE_TESTS=OFF
                   -DENABLE_ASSEMBLY=OFF          # nasm/yasm absent on this host
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
