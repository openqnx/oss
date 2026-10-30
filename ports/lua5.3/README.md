# lua5.3 5.3.6

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://www.lua.org/ftp/lua-5.3.6.tar.gz |
| SHA-256 | `fc5fd69bb8736323f026672b1b7235da613d7177e72558893a0bdcd320466d60` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add lua5.3
apk add lua5.3-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install lua5.3`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild lua5.3` performs exactly the same steps (2–5) in one command.

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
URL=https://www.lua.org/ftp/lua-5.3.6.tar.gz
mkdir -p dist src/lua5.3
curl -fL -o dist/$(basename $URL) $URL
echo "fc5fd69bb8736323f026672b1b7235da613d7177e72558893a0bdcd320466d60  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/lua5.3 --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/lua5.3 < ports/lua5.3/patches/0001-aports-lua-5.3-module_paths.patch
```

**[`0001-aports-lua-5.3-module_paths.patch`](patches/0001-aports-lua-5.3-module_paths.patch)** — QNX change from QNX's aports (extra/lua5.3/lua-5.3-module_paths.patch).

> Taken from qnx-ports/aports extra/lua5.3 -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `src/luaconf.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/lua5.3; B=build/$QNX_ARCH/lua5.3; mkdir -p $B
```

```bash
do_configure() { :; }
```

```bash
do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  local CFLAGS="-O2 -fPIC -D_QNX_SOURCE -DLUA_USE_POSIX -DLUA_USE_DLOPEN -I$S/src"
  for f in "$S"/src/*.c; do
    case "$(basename "$f")" in lua.c|luac.c|print.c) continue ;; esac
    $CC $CFLAGS -c "$f" -o "$(basename "${f%.c}").o" || return 1
  done
  $CC -shared -Wl,-soname,liblua5.3.so.0 -o liblua5.3.so.0.0.0 ./*.o -lm || return 1
  ln -sf liblua5.3.so.0.0.0 liblua5.3.so.0; ln -sf liblua5.3.so.0.0.0 liblua5.3.so
  "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs liblua5.3.a ./*.o
  $CC $CFLAGS -o lua5.3 "$S/src/lua.c" -L. -llua5.3 -lm || return 1
  # luac uses internal symbols, so it links the static library
  $CC $CFLAGS -o luac5.3 "$S/src/luac.c" $( [ -f "$S/src/print.c" ] && echo "$S/src/print.c" ) liblua5.3.a -lm
}
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/bin" "$STAGE$PREFIX/include/lua5.3" "$STAGE$PREFIX/lib/pkgconfig"
  install -m644 "$B"/liblua5.3.so.0.0.0 "$B"/liblua5.3.a "$STAGE$PREFIX/lib/"
  cp -a "$B"/liblua5.3.so.0 "$B"/liblua5.3.so "$STAGE$PREFIX/lib/"
  install -m755 "$B"/lua5.3 "$B"/luac5.3 "$STAGE$PREFIX/bin/"
  for h in lua.h luaconf.h lualib.h lauxlib.h lua.hpp; do [ -f "$S/src/$h" ] && install -m644 "$S/src/$h" "$STAGE$PREFIX/include/lua5.3/"; done
  cat > "$STAGE$PREFIX/lib/pkgconfig/lua5.3.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include/lua5.3
Name: Lua
Description: Lua language engine 5.3
Version: $VERSION
Libs: -L\${libdir} -llua5.3 -lm
Cflags: -I\${includedir}
PC
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Lua 5.3, alongside the default Lua (5.4): library liblua5.3, interpreter lua5.3, compiler luac5.3, headers in include/lua5.3. Version as in the QNX Everywhere 8.0.4r1 image.
- Lua ships a plain Makefile with hardcoded platform recipes and no QNX target; build the library and the two programs directly.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH lua5.3`.

### 5. What it installs

12 files under `/usr/local`: 3 shared-library entries, 2 programs, 5 headers.

```
/usr/local/lib/liblua5.3.so
/usr/local/lib/liblua5.3.so.0
/usr/local/lib/liblua5.3.so.0.0.0
/usr/local/bin/lua5.3
/usr/local/bin/luac5.3
```

The complete list is `stage/<arch>/.manifest/lua5.3` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa lua5.3
ls dists/8.0/*/ | grep '^lua5.3-'
```

This writes `lua5.3-<ver>-r0.tar.gz` (run time) and `lua5.3-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add lua5.3
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/lua5.3-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/lua5.3-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lua5.3 5.3.6-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lua5.3 5.3.6-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **5.3.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add lua5.3=<version>`), sources in `ported/lua5.3/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates lua5.3`; port one with `scripts/port-update lua5.3`.

<details><summary>recipe.sh</summary>

```bash
# Lua 5.3, alongside the default Lua (5.4): library liblua5.3, interpreter
# lua5.3, compiler luac5.3, headers in include/lua5.3. Version as in the QNX
# Everywhere 8.0.4r1 image.
VERSION=5.3.6
TARBALL=https://www.lua.org/ftp/lua-5.3.6.tar.gz
SHA256=fc5fd69bb8736323f026672b1b7235da613d7177e72558893a0bdcd320466d60
BUILDSYS=custom
# Lua ships a plain Makefile with hardcoded platform recipes and no QNX target;
# build the library and the two programs directly.
do_configure() { :; }
do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  local CFLAGS="-O2 -fPIC -D_QNX_SOURCE -DLUA_USE_POSIX -DLUA_USE_DLOPEN -I$S/src"
  for f in "$S"/src/*.c; do
    case "$(basename "$f")" in lua.c|luac.c|print.c) continue ;; esac
    $CC $CFLAGS -c "$f" -o "$(basename "${f%.c}").o" || return 1
  done
  $CC -shared -Wl,-soname,liblua5.3.so.0 -o liblua5.3.so.0.0.0 ./*.o -lm || return 1
  ln -sf liblua5.3.so.0.0.0 liblua5.3.so.0; ln -sf liblua5.3.so.0.0.0 liblua5.3.so
  "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs liblua5.3.a ./*.o
  $CC $CFLAGS -o lua5.3 "$S/src/lua.c" -L. -llua5.3 -lm || return 1
  # luac uses internal symbols, so it links the static library
  $CC $CFLAGS -o luac5.3 "$S/src/luac.c" $( [ -f "$S/src/print.c" ] && echo "$S/src/print.c" ) liblua5.3.a -lm
}
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/bin" "$STAGE$PREFIX/include/lua5.3" "$STAGE$PREFIX/lib/pkgconfig"
  install -m644 "$B"/liblua5.3.so.0.0.0 "$B"/liblua5.3.a "$STAGE$PREFIX/lib/"
  cp -a "$B"/liblua5.3.so.0 "$B"/liblua5.3.so "$STAGE$PREFIX/lib/"
  install -m755 "$B"/lua5.3 "$B"/luac5.3 "$STAGE$PREFIX/bin/"
  for h in lua.h luaconf.h lualib.h lauxlib.h lua.hpp; do [ -f "$S/src/$h" ] && install -m644 "$S/src/$h" "$STAGE$PREFIX/include/lua5.3/"; done
  cat > "$STAGE$PREFIX/lib/pkgconfig/lua5.3.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include/lua5.3
Name: Lua
Description: Lua language engine 5.3
Version: $VERSION
Libs: -L\${libdir} -llua5.3 -lm
Cflags: -I\${includedir}
PC
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
