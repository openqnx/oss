# lua 5.4.7

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://www.lua.org/ftp/lua-5.4.7.tar.gz |
| SHA-256 | `9fbf5e28ef86c69858f6d3d34eccc32e911c1a28b4120ff3e84aaa70cfbf1e30` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_lua`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add lua
apk add lua-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-lua && /usr/local/bin/qnx-port-tests/test_lua    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install lua`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild lua` performs exactly the same steps (2–5) in one command.

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
URL=https://www.lua.org/ftp/lua-5.4.7.tar.gz
mkdir -p dist src/lua
curl -fL -o dist/$(basename $URL) $URL
echo "9fbf5e28ef86c69858f6d3d34eccc32e911c1a28b4120ff3e84aaa70cfbf1e30  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/lua --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/lua; B=build/$QNX_ARCH/lua; mkdir -p $B
```

```bash
do_configure() { :; }
```

```bash
do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  local CFLAGS="-O2 -fPIC -D_QNX_SOURCE -DLUA_USE_POSIX -I$S/src"
  for f in "$S"/src/*.c; do
    case "$(basename "$f")" in lua.c|luac.c) continue ;; esac
    $CC $CFLAGS -c "$f" -o "$(basename "${f%.c}").o" || return 1
  done
  $CC -shared -Wl,-soname,liblua5.4.so.0 -o liblua5.4.so.0.0.0 ./*.o -lm || return 1
  ln -sf liblua5.4.so.0.0.0 liblua5.4.so.0; ln -sf liblua5.4.so.0.0.0 liblua5.4.so
  "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs liblua5.4.a ./*.o
}
```

```bash
do_install() {
  # Headers in include/lua5.4, as the lua5.1 and lua5.3 components (and
  # Debian) do: in include/ itself Lua 5.4's lua.h shadowed LuaJIT's
  # (include/luajit-2.1) for anything built against LuaJIT -- Neovim compiled
  # its Lua 5.3+ code paths and failed.
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include/lua5.4" "$STAGE$PREFIX/lib/pkgconfig"
  install -m644 "$B"/liblua5.4.so.0.0.0 "$B"/liblua5.4.a "$STAGE$PREFIX/lib/"
  cp -a "$B"/liblua5.4.so.0 "$B"/liblua5.4.so "$STAGE$PREFIX/lib/"
  install -m644 "$S"/src/lua.h "$S"/src/luaconf.h "$S"/src/lualib.h "$S"/src/lauxlib.h "$S"/src/lua.hpp "$STAGE$PREFIX/include/lua5.4/"
  cat > "$STAGE$PREFIX/lib/pkgconfig/lua5.4.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include/lua5.4
Name: Lua
Description: Lua language engine
Version: $VERSION
Libs: -L\${libdir} -llua5.4 -lm
Cflags: -I\${includedir}
PC
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Lua ships a plain Makefile with hardcoded platform recipes and no QNX target; build the library directly.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH lua`.

### 5. What it installs

10 files under `/usr/local`: 3 shared-library entries, 5 headers.

```
/usr/local/lib/liblua5.4.so
/usr/local/lib/liblua5.4.so.0
/usr/local/lib/liblua5.4.so.0.0.0
```

The complete list is `stage/<arch>/.manifest/lua` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa lua
ls dists/8.0/*/ | grep '^lua-'
```

This writes `lua-<ver>-r0.tar.gz` (run time) and `lua-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add lua
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/lua-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/lua-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_lua`](../../tests/src/) links lua the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-lua`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-lua
/usr/local/bin/qnx-port-tests/test_lua
```

```
== lua ==
  ok   ...
PASS lua
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_lua` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_lua`](../../tests/src/) -- a program linked against lua the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - luaL_newstate
   - run script that calls a C function
   - C function result reached Lua
   - script error is reported
   - string.format

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **5.4.7**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add lua=<version>`), sources in `ported/lua/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates lua`; port one with `scripts/port-update lua`.

<details><summary>recipe.sh</summary>

```bash
VERSION=5.4.7
TARBALL=https://www.lua.org/ftp/lua-5.4.7.tar.gz
SHA256=9fbf5e28ef86c69858f6d3d34eccc32e911c1a28b4120ff3e84aaa70cfbf1e30
BUILDSYS=custom
# Lua ships a plain Makefile with hardcoded platform recipes and no QNX target;
# build the library directly.
do_configure() { :; }
do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  local CFLAGS="-O2 -fPIC -D_QNX_SOURCE -DLUA_USE_POSIX -I$S/src"
  for f in "$S"/src/*.c; do
    case "$(basename "$f")" in lua.c|luac.c) continue ;; esac
    $CC $CFLAGS -c "$f" -o "$(basename "${f%.c}").o" || return 1
  done
  $CC -shared -Wl,-soname,liblua5.4.so.0 -o liblua5.4.so.0.0.0 ./*.o -lm || return 1
  ln -sf liblua5.4.so.0.0.0 liblua5.4.so.0; ln -sf liblua5.4.so.0.0.0 liblua5.4.so
  "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs liblua5.4.a ./*.o
}
do_install() {
  # Headers in include/lua5.4, as the lua5.1 and lua5.3 components (and
  # Debian) do: in include/ itself Lua 5.4's lua.h shadowed LuaJIT's
  # (include/luajit-2.1) for anything built against LuaJIT -- Neovim compiled
  # its Lua 5.3+ code paths and failed.
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include/lua5.4" "$STAGE$PREFIX/lib/pkgconfig"
  install -m644 "$B"/liblua5.4.so.0.0.0 "$B"/liblua5.4.a "$STAGE$PREFIX/lib/"
  cp -a "$B"/liblua5.4.so.0 "$B"/liblua5.4.so "$STAGE$PREFIX/lib/"
  install -m644 "$S"/src/lua.h "$S"/src/luaconf.h "$S"/src/lualib.h "$S"/src/lauxlib.h "$S"/src/lua.hpp "$STAGE$PREFIX/include/lua5.4/"
  cat > "$STAGE$PREFIX/lib/pkgconfig/lua5.4.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include/lua5.4
Name: Lua
Description: Lua language engine
Version: $VERSION
Libs: -L\${libdir} -llua5.4 -lm
Cflags: -I\${includedir}
PC
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
