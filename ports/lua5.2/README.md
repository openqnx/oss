# lua5.2 5.2.4

> Lua 5.2 scripting language interpreter and library

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://www.lua.org/ftp/lua-5.2.4.tar.gz |
| SHA-256 | `b9e2e4aad6789b3b63a056d442f7b39f0ecfca3ae0f1fc0ae4e9614401b69f4b` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_lua5_2`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `lua5.2` (programs, shared libraries, run-time data) and `lua5.2-dev` (headers, pkg-config/CMake files, static libraries; needs `lua5.2`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`lua5.2`](https://openqnx.github.io/oss/package/x86_64/lua5.2.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lua5.2.html)) · [`lua5.2-dev`](https://openqnx.github.io/oss/package/x86_64/lua5.2-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lua5.2-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add lua5.2` (dependencies come along); one version: `apk add lua5.2=<version>` | `apk del lua5.2` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install lua5.2` (dependencies first) | `oss-get remove lua5.2` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`lua5.2-dev` the same way, by its own name (`apk add lua5.2-dev`, `oss-get install lua5.2-dev`, by hand with its own file and list `lua5.2-dev.list`); remove it before `lua5.2`, which it needs.

```sh
# on the QNX target, as root
apk add lua5.2 lua5.2-dev
apk add qnx-test-lua5-2 && /usr/local/bin/qnx-port-tests/test_lua5_2    # its on-target test, from tests-8.0-<arch>
apk del lua5.2 lua5.2-dev qnx-test-lua5-2
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/lua5.2.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/lua5.2.list
rm var/lib/manual/lua5.2.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild lua5.2` performs exactly the same steps (2–5) in one command.

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
URL=https://www.lua.org/ftp/lua-5.2.4.tar.gz
mkdir -p dist src/lua5.2
curl -fL -o dist/$(basename $URL) $URL
echo "b9e2e4aad6789b3b63a056d442f7b39f0ecfca3ae0f1fc0ae4e9614401b69f4b  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/lua5.2 --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/lua5.2; B=build/$QNX_ARCH/lua5.2; mkdir -p $B
```

```bash
do_configure() { :; }
```

```bash
do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  # LUA_COMPAT_ALL: what upstream's Makefile builds 5.2 with (the 5.1
  # functions such as unpack, loadstring, math.pow, module()).
  local CFLAGS="-O2 -fPIC -D_QNX_SOURCE -DLUA_USE_POSIX -DLUA_USE_DLOPEN -DLUA_COMPAT_ALL -I$S/src"
  for f in "$S"/src/*.c; do
    case "$(basename "$f")" in lua.c|luac.c|print.c) continue ;; esac
    $CC $CFLAGS -c "$f" -o "$(basename "${f%.c}").o" || return 1
  done
  $CC -shared -Wl,-soname,liblua5.2.so.0 -o liblua5.2.so.0.0.0 ./*.o -lm || return 1
  ln -sf liblua5.2.so.0.0.0 liblua5.2.so.0; ln -sf liblua5.2.so.0.0.0 liblua5.2.so
  "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs liblua5.2.a ./*.o
  $CC $CFLAGS -o lua5.2 "$S/src/lua.c" -L. -llua5.2 -lm || return 1
  # luac uses internal symbols, so it links the static library
  $CC $CFLAGS -o luac5.2 "$S/src/luac.c" $( [ -f "$S/src/print.c" ] && echo "$S/src/print.c" ) liblua5.2.a -lm
}
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/bin" "$STAGE$PREFIX/include/lua5.2" "$STAGE$PREFIX/lib/pkgconfig"
  install -m644 "$B"/liblua5.2.so.0.0.0 "$B"/liblua5.2.a "$STAGE$PREFIX/lib/"
  cp -a "$B"/liblua5.2.so.0 "$B"/liblua5.2.so "$STAGE$PREFIX/lib/"
  install -m755 "$B"/lua5.2 "$B"/luac5.2 "$STAGE$PREFIX/bin/"
  for h in lua.h luaconf.h lualib.h lauxlib.h lua.hpp; do [ -f "$S/src/$h" ] && install -m644 "$S/src/$h" "$STAGE$PREFIX/include/lua5.2/"; done
  cat > "$STAGE$PREFIX/lib/pkgconfig/lua5.2.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include/lua5.2
Name: Lua
Description: Lua language engine 5.2
Version: $VERSION
Libs: -L\${libdir} -llua5.2 -lm
Cflags: -I\${includedir}
PC
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Lua 5.2, alongside the default Lua (5.4) and 5.1, 5.3: library liblua5.2, interpreter lua5.2, compiler luac5.2, headers in include/lua5.2. Version as in the QNX package list.
- Lua ships a plain Makefile with hardcoded platform recipes and no QNX target; build the library and the two programs directly.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH lua5.2`.

### 5. What it installs

12 files under `/usr/local`: 3 shared-library entries, 2 programs, 5 headers.

```
/usr/local/lib/liblua5.2.so
/usr/local/lib/liblua5.2.so.0
/usr/local/lib/liblua5.2.so.0.0.0
/usr/local/bin/lua5.2
/usr/local/bin/luac5.2
```

The complete list is `stage/<arch>/.manifest/lua5.2` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa lua5.2
ls dists/8.0/*/ | grep '^lua5.2-'
```

This writes `lua5.2-<ver>-r0.tar.gz` (run time) and `lua5.2-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add lua5.2
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/lua5.2-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/lua5.2-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_lua5_2`](../../tests/src/) links lua5.2 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-lua5-2`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-lua5-2
/usr/local/bin/qnx-port-tests/test_lua5_2
```

```
== lua5.2 ==
  ok   ...
PASS lua5.2
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_lua5_2` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_lua5_2`](../../tests/src/) -- a program linked against lua5.2 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - luaL_newstate
   - scratch directory
   - lua5.2 runs a script with the known output
   - lua5.2 -e
   - luac5.2 compiles the script
   - bytecode has the Lua 5.2 header (ESC Lua 0x52)
   - lua5.2 runs the bytecode with the same output
   - luac5.2 -p reports a syntax error

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> lua5.2`.

## Versions

This recipe builds **5.2.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add lua5.2=<version>`), sources in `ported/lua5.2/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates lua5.2`; port one with `scripts/port-update lua5.2`.

<details><summary>recipe.sh</summary>

```bash
# Lua 5.2, alongside the default Lua (5.4) and 5.1, 5.3: library liblua5.2, interpreter
# lua5.2, compiler luac5.2, headers in include/lua5.2. Version as in the QNX
# package list.
VERSION=5.2.4
TARBALL=https://www.lua.org/ftp/lua-5.2.4.tar.gz
SHA256=b9e2e4aad6789b3b63a056d442f7b39f0ecfca3ae0f1fc0ae4e9614401b69f4b
BUILDSYS=custom
# Lua ships a plain Makefile with hardcoded platform recipes and no QNX target;
# build the library and the two programs directly.
do_configure() { :; }
do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  # LUA_COMPAT_ALL: what upstream's Makefile builds 5.2 with (the 5.1
  # functions such as unpack, loadstring, math.pow, module()).
  local CFLAGS="-O2 -fPIC -D_QNX_SOURCE -DLUA_USE_POSIX -DLUA_USE_DLOPEN -DLUA_COMPAT_ALL -I$S/src"
  for f in "$S"/src/*.c; do
    case "$(basename "$f")" in lua.c|luac.c|print.c) continue ;; esac
    $CC $CFLAGS -c "$f" -o "$(basename "${f%.c}").o" || return 1
  done
  $CC -shared -Wl,-soname,liblua5.2.so.0 -o liblua5.2.so.0.0.0 ./*.o -lm || return 1
  ln -sf liblua5.2.so.0.0.0 liblua5.2.so.0; ln -sf liblua5.2.so.0.0.0 liblua5.2.so
  "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs liblua5.2.a ./*.o
  $CC $CFLAGS -o lua5.2 "$S/src/lua.c" -L. -llua5.2 -lm || return 1
  # luac uses internal symbols, so it links the static library
  $CC $CFLAGS -o luac5.2 "$S/src/luac.c" $( [ -f "$S/src/print.c" ] && echo "$S/src/print.c" ) liblua5.2.a -lm
}
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/bin" "$STAGE$PREFIX/include/lua5.2" "$STAGE$PREFIX/lib/pkgconfig"
  install -m644 "$B"/liblua5.2.so.0.0.0 "$B"/liblua5.2.a "$STAGE$PREFIX/lib/"
  cp -a "$B"/liblua5.2.so.0 "$B"/liblua5.2.so "$STAGE$PREFIX/lib/"
  install -m755 "$B"/lua5.2 "$B"/luac5.2 "$STAGE$PREFIX/bin/"
  for h in lua.h luaconf.h lualib.h lauxlib.h lua.hpp; do [ -f "$S/src/$h" ] && install -m644 "$S/src/$h" "$STAGE$PREFIX/include/lua5.2/"; done
  cat > "$STAGE$PREFIX/lib/pkgconfig/lua5.2.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include/lua5.2
Name: Lua
Description: Lua language engine 5.2
Version: $VERSION
Libs: -L\${libdir} -llua5.2 -lm
Cflags: -I\${includedir}
PC
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
