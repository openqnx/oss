# candb 0.1-git-20210625

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/GENIVI/CANdb.git |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install candb
oss-get install candb-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild candb` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/GENIVI/CANdb.git src/candb
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/candb < ports/candb/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/candb < ports/candb/patches/0002-bundled-gtest-only-for-tests.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — add support for QNX 7.1/8.0

> Imported from qnx-ports/CANdb (150c42a8cdfc0748915f09d2cd0c5a68591263a7...qnx-master); original authorship kept.
>
>
> extend opendbc_tests + 8 additional dbc files
> ---
>  .gitignore                            |  3 ++
>  src/dbc_grammar.peg                   |  8 ++--
>  tests/CMakeLists.txt                  | 41 +++++++++++++++++-
>  tests/opendbc_tests.cpp               | 60 ++++++++++++++++++++++++++-
>  tests/opendbc_tests_expected_data.hpp |  2 +-
>  5 files changed, 107 insertions(+), 7 deletions(-)

Touches: `.gitignore`, `src/dbc_grammar.peg`, `tests/CMakeLists.txt`, `tests/opendbc_tests.cpp`, `tests/opendbc_tests_expected_data.hpp` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-bundled-gtest-only-for-tests.patch`](patches/0002-bundled-gtest-only-for-tests.patch)** — Build the bundled googletest only when tests are enabled.

> 3rdParty/CMakeLists.txt always builds the bundled (old) googletest and
> googlemock. With CANDB__WITH_TESTS=OFF nothing uses them, and in a tree that
> also has a current googletest installed the old gmock picks up the new gtest
> headers and fails to compile.

Touches: `3rdParty/CMakeLists.txt` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/candb -B build/$QNX_ARCH/candb -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCANDB__WITH_TESTS=OFF \
      -DCANDB__WITH_TOOLS=ON \
      -DBUILD_SHARED_LIBS=ON
cmake --build build/$QNX_ARCH/candb -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/candb
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH candb`.

### 5. What it installs

4 files under `/usr/local`, 1 headers.

The complete list is `stage/<arch>/.manifest/candb` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH candb
ls repo/8.0/*/ | grep '^candb-'
```

This writes `candb-<ver>-r0.tar.gz` (run time) and `candb-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install candb
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/candb-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/candb-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.1-git-20210625; GIT=https://github.com/GENIVI/CANdb.git; COMMIT=150c42a8cdfc0748915f09d2cd0c5a68591263a7
CMAKE_ARGS_EXTRA=( -DCANDB__WITH_TESTS=OFF -DCANDB__WITH_TOOLS=ON -DBUILD_SHARED_LIBS=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
