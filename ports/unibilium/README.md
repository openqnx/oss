# unibilium 2.1.2

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/neovim/unibilium.git (tag `v2.1.2`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_unibilium`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install unibilium
oss-get install unibilium-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild unibilium` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2.1.2 --recurse-submodules --shallow-submodules \
    https://github.com/neovim/unibilium.git src/unibilium
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/unibilium < ports/unibilium/patches/0001-cmake-libtool-compatible-soname.patch
```

**[`0001-cmake-libtool-compatible-soname.patch`](patches/0001-cmake-libtool-compatible-soname.patch)** — Give the shared library the soname of the libtool build: libunibilium.so.4.

> The CMake build names the library version "${VERSION_MAJOR}.${VERSION_MINOR}",
> two variables nothing defines (project() sets PROJECT_VERSION_MAJOR), so the
> library came out as "libunibilium.so.." with that as its soname, and every
> program linking it -- neovim -- needed a file called "libunibilium.so..".
> Use what the Makefile build of the same release produces (-version-info
> 4:1:0): libunibilium.so.4.0.1, soname libunibilium.so.4, as distributions
> ship it.

Touches: `CMakeLists.txt` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/unibilium -B build/$QNX_ARCH/unibilium -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DTERMINFO_DIRS=/usr/local/share/terminfo:/usr/share/terminfo:/etc/terminfo:/usr/lib/terminfo
cmake --build build/$QNX_ARCH/unibilium -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/unibilium
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- TERMINFO_DIRS: the compiled-in search path. Left alone, CMakeLists asks the BUILD HOST's ncurses6-config and bakes in the host's directories (/etc/terminfo:/lib/terminfo:/usr/share/terminfo), none of which holds this tree's database on the target: the ncurses component installs it in /usr/local/share/terminfo. TERMINFO / TERMINFO_DIRS still override it.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH unibilium`.

### 5. What it installs

4 files under `/usr/local`: 3 shared-library entries, 1 headers.

```
/usr/local/lib/libunibilium.so
/usr/local/lib/libunibilium.so.4
/usr/local/lib/libunibilium.so.4.0.1
```

The complete list is `stage/<arch>/.manifest/unibilium` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH unibilium
ls repo/8.0/*/ | grep '^unibilium-'
```

This writes `unibilium-<ver>-r0.tar.gz` (run time) and `unibilium-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install unibilium
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/unibilium-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/unibilium-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_unibilium`](../../tests/src/) links unibilium the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_unibilium root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_unibilium'
```

```
== unibilium ==
  ok   ...
PASS unibilium
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.1.2; GIT=https://github.com/neovim/unibilium.git; REF=v2.1.2
BUILDSYS=cmake
# TERMINFO_DIRS: the compiled-in search path. Left alone, CMakeLists asks the
# BUILD HOST's ncurses6-config and bakes in the host's directories
# (/etc/terminfo:/lib/terminfo:/usr/share/terminfo), none of which holds this
# tree's database on the target: the ncurses component installs it in
# /usr/local/share/terminfo. TERMINFO / TERMINFO_DIRS still override it.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON
                   -DTERMINFO_DIRS=/usr/local/share/terminfo:/usr/share/terminfo:/etc/terminfo:/usr/lib/terminfo )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
