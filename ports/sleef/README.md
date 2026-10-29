# sleef 3.8-git-20250214

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/shibatch/sleef.git |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install sleef
oss-get install sleef-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild sleef` performs exactly the same steps (2–5) in one command.

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
    https://github.com/shibatch/sleef.git src/sleef
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/sleef < ports/sleef/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add QNX support.

> Imported from qnx-ports/sleef (b8abbd56df03...qnx_master); original authorship kept.
>
>
> ---
>  Configure.cmake        | 16 ++++++++++++----
>  src/dft/CMakeLists.txt |  4 ++--
>  2 files changed, 14 insertions(+), 6 deletions(-)

Touches: `Configure.cmake`, `src/dft/CMakeLists.txt` · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/sleef; B=build/$QNX_ARCH/sleef; mkdir -p $B
```

```bash
do_configure() {
  local nat="$B/native"; mkdir -p "$nat"
  env -u CC -u CXX cmake -S "$S" -B "$nat" -G Ninja -DCMAKE_BUILD_TYPE=Release -DSLEEF_BUILD_TESTS=OFF \
      -DSLEEF_BUILD_DFT=OFF -DSLEEF_BUILD_QUAD=OFF -DSLEEF_BUILD_GNUABI_LIBS=OFF && ninja -C "$nat" || return 1
  cmake -S "$S" -B "$B/target" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$arch.cmake" -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="$PREFIX" -DNATIVE_BUILD_DIR="$nat" -DSLEEF_BUILD_TESTS=OFF -DSLEEF_BUILD_SHARED_LIBS=ON \
      -DSLEEF_BUILD_DFT=OFF -DSLEEF_BUILD_QUAD=OFF -DSLEEF_BUILD_GNUABI_LIBS=OFF -DSLEEF_BUILD_INLINE_HEADERS=OFF
}
```

```bash
do_build() { ninja -C "$B/target"; }
```

```bash
do_install() { DESTDIR="$STAGE" cmake --install "$B/target"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- SLEEF generates parts of itself with helper programs that must run on the build host: a native build of those helpers is made first (do_configure).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH sleef`.

### 5. What it installs

9 files under `/usr/local`: 3 shared-library entries, 1 headers.

```
/usr/local/lib/libsleef.so
/usr/local/lib/libsleef.so.3
/usr/local/lib/libsleef.so.3.9.0
```

The complete list is `stage/<arch>/.manifest/sleef` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH sleef
ls repo/8.0/*/ | grep '^sleef-'
```

This writes `sleef-<ver>-r0.tar.gz` (run time) and `sleef-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install sleef
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/sleef-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/sleef-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.8-git-20250214; GIT=https://github.com/shibatch/sleef.git; COMMIT=b8abbd56df03eb7cc65b8025495b955ce5336973
# SLEEF generates parts of itself with helper programs that must run on the
# build host: a native build of those helpers is made first (do_configure).
do_configure() {
  local nat="$B/native"; mkdir -p "$nat"
  env -u CC -u CXX cmake -S "$S" -B "$nat" -G Ninja -DCMAKE_BUILD_TYPE=Release -DSLEEF_BUILD_TESTS=OFF \
      -DSLEEF_BUILD_DFT=OFF -DSLEEF_BUILD_QUAD=OFF -DSLEEF_BUILD_GNUABI_LIBS=OFF && ninja -C "$nat" || return 1
  cmake -S "$S" -B "$B/target" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$arch.cmake" -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="$PREFIX" -DNATIVE_BUILD_DIR="$nat" -DSLEEF_BUILD_TESTS=OFF -DSLEEF_BUILD_SHARED_LIBS=ON \
      -DSLEEF_BUILD_DFT=OFF -DSLEEF_BUILD_QUAD=OFF -DSLEEF_BUILD_GNUABI_LIBS=OFF -DSLEEF_BUILD_INLINE_HEADERS=OFF
}
do_build() { ninja -C "$B/target"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B/target"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
