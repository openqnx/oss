# luv 1.51.0-1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/luvit/luv.git (tag `1.51.0-1`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`libuv`](../libuv/) [`luajit`](../luajit/) |
| Run-time dependencies | [`libuv`](../libuv/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add luv
apk add luv-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install luv`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild luv` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libuv luajit
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.51.0-1 --recurse-submodules --shallow-submodules \
    https://github.com/luvit/luv.git src/luv
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/luv -B build/$QNX_ARCH/luv -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_MODULE=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_STATIC_LIBS=OFF \
      -DWITH_SHARED_LIBUV=ON \
      -DWITH_LUA_ENGINE=LuaJIT \
      -DLUA_BUILD_TYPE=System \
      -DLIBUV_INCLUDE_DIR="$STAGE$PREFIX/include" \
      -DLIBUV_LIBRARIES="$STAGE$PREFIX/lib/libuv.so" \
      -DLUAJIT_INCLUDE_DIR="$STAGE$PREFIX/include/luajit-2.1" \
      -DLUAJIT_LIBRARIES="$STAGE$PREFIX/lib/libluajit-5.1.so"
cmake --build build/$QNX_ARCH/luv -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/luv
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libuv bindings for Lua -- the event loop under Neovim's Lua API.  Built as a shared library against the staged libuv and LuaJIT rather than its bundled submodule copies, so there is exactly one libuv in the tree.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH luv`.

### 5. What it installs

8 files under `/usr/local`: 3 shared-library entries, 4 headers.

```
/usr/local/lib/libluv.so
/usr/local/lib/libluv.so.1
/usr/local/lib/libluv.so.1.51.0
```

The complete list is `stage/<arch>/.manifest/luv` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa luv
ls dists/8.0/*/ | grep '^luv-'
```

This writes `luv-<ver>-r0.tar.gz` (run time) and `luv-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add luv
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/luv-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/luv-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | luv 1.51.0.1-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.51.0-1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add luv=<version>`), sources in `ported/luv/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates luv`; port one with `scripts/port-update luv`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.51.0-1; GIT=https://github.com/luvit/luv.git; REF=1.51.0-1
DEPS="libuv luajit"
BUILDSYS=cmake
# libuv bindings for Lua -- the event loop under Neovim's Lua API.  Built as a
# shared library against the staged libuv and LuaJIT rather than its bundled
# submodule copies, so there is exactly one libuv in the tree.
CMAKE_ARGS_EXTRA=( -DBUILD_MODULE=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF
                   -DWITH_SHARED_LIBUV=ON -DWITH_LUA_ENGINE=LuaJIT
                   -DLUA_BUILD_TYPE=System
                   -DLIBUV_INCLUDE_DIR="$STAGE$PREFIX/include"
                   -DLIBUV_LIBRARIES="$STAGE$PREFIX/lib/libuv.so"
                   -DLUAJIT_INCLUDE_DIR="$STAGE$PREFIX/include/luajit-2.1"
                   -DLUAJIT_LIBRARIES="$STAGE$PREFIX/lib/libluajit-5.1.so" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
