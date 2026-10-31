# neovim 0.12.3

> Vim-based extensible text editor

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/neovim/neovim.git (tag `v0.12.3`) |
| Build system | custom (see recipe) |
| QNX patches | 4 |
| Build dependencies | [`luajit`](../luajit/) [`libuv`](../libuv/) [`utf8proc`](../utf8proc/) [`tree-sitter`](../tree-sitter/) [`unibilium`](../unibilium/) [`lua-lpeg`](../lua-lpeg/) [`luv`](../luv/) [`libiconv`](../libiconv/) [`gettext`](../gettext/) |
| Run-time dependencies | [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`luv`](../luv/) [`tree-sitter`](../tree-sitter/) [`unibilium`](../unibilium/) [`utf8proc`](../utf8proc/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_neovim`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `neovim` (programs, shared libraries, run-time data) and `neovim-dev` (headers, pkg-config/CMake files, static libraries; needs `neovim`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`neovim`](https://openqnx.github.io/oss/package/x86_64/neovim.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/neovim.html)) · [`neovim-dev`](https://openqnx.github.io/oss/package/x86_64/neovim-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/neovim-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add neovim` (dependencies come along); one version: `apk add neovim=<version>` | `apk del neovim` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install neovim` (dependencies first) | `oss-get remove neovim` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`neovim-dev` the same way, by its own name (`apk add neovim-dev`, `oss-get install neovim-dev`, by hand with its own file and list `neovim-dev.list`); remove it before `neovim`, which it needs.

```sh
# on the QNX target, as root
apk add neovim neovim-dev
apk add qnx-test-neovim && /usr/local/bin/qnx-port-tests/test_neovim    # its on-target test
apk del neovim neovim-dev qnx-test-neovim
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/neovim.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/neovim.list
rm var/lib/manual/neovim.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa neovim
ls dists/8.0/*/ | grep '^neovim-'
```

This writes `neovim-<ver>-r0.tar.gz` (run time) and `neovim-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add neovim
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/neovim-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/neovim-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_neovim`](../../tests/src/) links neovim the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-neovim`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-neovim
/usr/local/bin/qnx-port-tests/test_neovim
```

```
== neovim ==
  ok   ...
PASS neovim
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_neovim` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_neovim`](../../tests/src/) -- a program linked against neovim the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | neovim 0.12.3-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/neovim-0.12.3-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | neovim 0.12.3-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/neovim-0.12.3-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | neovim 0.12.3-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/neovim-0.12.3-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | neovim 0.12.3-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/neovim-0.12.3-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.12.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add neovim=<version>`), sources in `ported/neovim/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates neovim`; port one with `scripts/port-update neovim`.

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
