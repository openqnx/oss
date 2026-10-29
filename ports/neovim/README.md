# neovim 0.12.3

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/neovim/neovim.git (tag `v0.12.3`) |
| Build system | custom (see recipe) |
| QNX patches | 4 |
| Build dependencies | [`luajit`](../luajit/) [`libuv`](../libuv/) [`utf8proc`](../utf8proc/) [`tree-sitter`](../tree-sitter/) [`unibilium`](../unibilium/) [`lua-lpeg`](../lua-lpeg/) [`luv`](../luv/) [`libiconv`](../libiconv/) [`gettext`](../gettext/) |
| Run-time dependencies | [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`luv`](../luv/) [`tree-sitter`](../tree-sitter/) [`unibilium`](../unibilium/) [`utf8proc`](../utf8proc/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install neovim
oss-get install neovim-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild neovim` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (11 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild luajit libuv utf8proc tree-sitter unibilium lua-lpeg luv libiconv gettext
```

<details><summary>Full build chain (11)</summary>

[`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libuv`](../libuv/) [`lua-lpeg`](../lua-lpeg/) [`luajit`](../luajit/) [`luv`](../luv/) [`ncurses`](../ncurses/) [`tree-sitter`](../tree-sitter/) [`unibilium`](../unibilium/) [`utf8proc`](../utf8proc/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.12.3 --recurse-submodules --shallow-submodules \
    https://github.com/neovim/neovim.git src/neovim
```

### 3. Apply the QNX patches

4 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/neovim < ports/neovim/patches/0001-aports-qnx-endian.patch
patch -p1 -d src/neovim < ports/neovim/patches/0002-aports-qnx-forkpty.patch
patch -p1 -d src/neovim < ports/neovim/patches/0003-aports-qnx-remove-util.patch
patch -p1 -d src/neovim < ports/neovim/patches/0004-aports-qnx-define.patch
```

**[`0001-aports-qnx-endian.patch`](patches/0001-aports-qnx-endian.patch)** — QNX build fix from QNX's aports (extra/neovim/qnx-endian.patch).

> Taken from qnx-ports/aports extra/neovim, the APKBUILD the QNX Everywhere image's
> package is built from.

Touches: `cmake.config/CMakeLists.txt` · Upstream status: QNX-specific

**[`0002-aports-qnx-forkpty.patch`](patches/0002-aports-qnx-forkpty.patch)** — QNX build fix from QNX's aports (extra/neovim/qnx-forkpty.patch).

> Taken from qnx-ports/aports extra/neovim, the APKBUILD the QNX Everywhere image's
> package is built from.

Touches: `src/nvim/os/pty_proc_unix.c` · Upstream status: QNX-specific

**[`0003-aports-qnx-remove-util.patch`](patches/0003-aports-qnx-remove-util.patch)** — QNX build fix from QNX's aports (extra/neovim/qnx-remove-util.patch).

> Taken from qnx-ports/aports extra/neovim, the APKBUILD the QNX Everywhere image's
> package is built from.

Touches: `src/nvim/CMakeLists.txt` · Upstream status: QNX-specific

**[`0004-aports-qnx-define.patch`](patches/0004-aports-qnx-define.patch)** — QNX build fix from QNX's aports (extra/neovim/qnx-define.patch).

> Taken from qnx-ports/aports extra/neovim, the APKBUILD the QNX Everywhere image's
> package is built from.

Touches: `test/unit/preprocess.lua` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/neovim; B=build/$QNX_ARCH/neovim; mkdir -p $B
```

```bash
do_configure() {
  local H="$BUILD_DIR/host/neovim-$VERSION-deps" N="$BUILD_DIR/host/neovim-$VERSION-nlua0"
  if [ ! -x "$H/usr/bin/luajit" ]; then
    _host cmake -S "$S/cmake.deps" -B "$H" -G Ninja -DUSE_BUNDLED=ON -DUSE_BUNDLED_GETTEXT=OFF -DUSE_BUNDLED_LIBICONV=OFF \
      && _host ninja -C "$H" || return 1
  fi
  if [ ! -f "$N/lib/libnlua0.so" ] || [ ! -x "$N/bin/nvim" ]; then
    _host cmake -S "$S" -B "$N" -G Ninja -DCMAKE_BUILD_TYPE=Release -DDEPS_PREFIX="$H/usr" -DENABLE_TRANSLATIONS=OFF \
      && _host ninja -C "$N" nlua0 nvim_bin || return 1
  fi
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCI_BUILD=OFF -DENABLE_TRANSLATIONS=OFF \
    -DLUA_PRG="$H/usr/bin/luajit" -DLUA_GEN_PRG="$H/usr/bin/luajit" -DNLUA0_HOST_PRG="$N/lib/libnlua0.so" -DNVIM_HOST_PRG="$N/bin/nvim" \
    -DLPEG_LIBRARY="$STAGE$PREFIX/lib/liblpeg.a"
}
```

```bash
do_build() { cmake --build "$B" -j"$(nproc)"; }
```

```bash
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Neovim with the QNX changes of QNX's aports (patches/), using this tree's libraries (USE_BUNDLED=OFF) as the QNX Everywhere image does. Neovim generates sources with Lua scripts while it builds, run by the host LuaJIT with "nlua0" -- a module (LPeg, mpack, ...) that must therefore be built for the *build host* as well. 0.12 takes a host-built one through NLUA0_HOST_PRG; without it, the cross build uses its own QNX nlua0 and the host LuaJIT cannot load it ("libsocket.so.4: cannot open shared object"). So, on the build host: cmake.deps makes Neovim's bundled dependencies, then a native configure of Neovim builds the nlua0 target against them -- and a host nvim, which the build runs to generate the help tags (NVIM_HOST_PRG).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH neovim`.

### 5. What it installs

2101 files under `/usr/local`, 1 programs.

```
/usr/local/bin/nvim
```

The complete list is `stage/<arch>/.manifest/neovim` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH neovim
ls repo/8.0/*/ | grep '^neovim-'
```

This writes `neovim-<ver>-r0.tar.gz` (run time) and `neovim-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install neovim
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/neovim-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/neovim-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.12.3; GIT=https://github.com/neovim/neovim.git; REF=v0.12.3
# Neovim with the QNX changes of QNX's aports (patches/), using this tree's
# libraries (USE_BUNDLED=OFF) as the QNX Everywhere image does.
#
# Neovim generates sources with Lua scripts while it builds, run by the host
# LuaJIT with "nlua0" -- a module (LPeg, mpack, ...) that must therefore be
# built for the *build host* as well. 0.12 takes a host-built one through
# NLUA0_HOST_PRG; without it, the cross build uses its own QNX nlua0 and the
# host LuaJIT cannot load it ("libsocket.so.4: cannot open shared object").
# So, on the build host: cmake.deps makes Neovim's bundled dependencies, then
# a native configure of Neovim builds the nlua0 target against them -- and a
# host nvim, which the build runs to generate the help tags (NVIM_HOST_PRG).
DEPS="luajit libuv utf8proc tree-sitter unibilium lua-lpeg luv libiconv gettext"
BUILDSYS=custom
_host() { env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u LDFLAGS -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH "$@"; }
do_configure() {
  local H="$BUILD_DIR/host/neovim-$VERSION-deps" N="$BUILD_DIR/host/neovim-$VERSION-nlua0"
  if [ ! -x "$H/usr/bin/luajit" ]; then
    _host cmake -S "$S/cmake.deps" -B "$H" -G Ninja -DUSE_BUNDLED=ON -DUSE_BUNDLED_GETTEXT=OFF -DUSE_BUNDLED_LIBICONV=OFF \
      && _host ninja -C "$H" || return 1
  fi
  if [ ! -f "$N/lib/libnlua0.so" ] || [ ! -x "$N/bin/nvim" ]; then
    _host cmake -S "$S" -B "$N" -G Ninja -DCMAKE_BUILD_TYPE=Release -DDEPS_PREFIX="$H/usr" -DENABLE_TRANSLATIONS=OFF \
      && _host ninja -C "$N" nlua0 nvim_bin || return 1
  fi
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCI_BUILD=OFF -DENABLE_TRANSLATIONS=OFF \
    -DLUA_PRG="$H/usr/bin/luajit" -DLUA_GEN_PRG="$H/usr/bin/luajit" -DNLUA0_HOST_PRG="$N/lib/libnlua0.so" -DNVIM_HOST_PRG="$N/bin/nvim" \
    -DLPEG_LIBRARY="$STAGE$PREFIX/lib/liblpeg.a"
}
do_build() { cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
