# lua-lpeg 1.1.0

> Parsing Expression Grammars (LPeg) module for Lua

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://www.inf.puc-rio.br/~roberto/lpeg/lpeg-1.1.0.tar.gz |
| SHA-256 | `4b155d67d2246c1ffa7ad7bc466c1ea899bbc40fef0257cc9c03cecbaed4352a` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`luajit`](../luajit/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_lua_lpeg`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `lua-lpeg` (programs, shared libraries, run-time data) and `lua-lpeg-dev` (headers, pkg-config/CMake files, static libraries; needs `lua-lpeg`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`lua-lpeg`](https://openqnx.github.io/oss/package/x86_64/lua-lpeg.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lua-lpeg.html)) · [`lua-lpeg-dev`](https://openqnx.github.io/oss/package/x86_64/lua-lpeg-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lua-lpeg-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add lua-lpeg` (dependencies come along); one version: `apk add lua-lpeg=<version>` | `apk del lua-lpeg` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install lua-lpeg` (dependencies first) | `oss-get remove lua-lpeg` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`lua-lpeg-dev` the same way, by its own name (`apk add lua-lpeg-dev`, `oss-get install lua-lpeg-dev`, by hand with its own file and list `lua-lpeg-dev.list`); remove it before `lua-lpeg`, which it needs.

```sh
# on the QNX target, as root
apk add lua-lpeg lua-lpeg-dev
apk add qnx-test-lua-lpeg && /usr/local/bin/qnx-port-tests/test_lua_lpeg    # its on-target test, from tests-8.0-<arch>
apk del lua-lpeg lua-lpeg-dev qnx-test-lua-lpeg
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/lua-lpeg.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/lua-lpeg.list
rm var/lib/manual/lua-lpeg.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild lua-lpeg` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild luajit
```

### 2. Get the upstream source

```bash
URL=https://www.inf.puc-rio.br/~roberto/lpeg/lpeg-1.1.0.tar.gz
mkdir -p dist src/lua-lpeg
curl -fL -o dist/$(basename $URL) $URL
echo "4b155d67d2246c1ffa7ad7bc466c1ea899bbc40fef0257cc9c03cecbaed4352a  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/lua-lpeg --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/lua-lpeg; B=build/$QNX_ARCH/lua-lpeg; mkdir -p $B
```

```bash
do_build() { cd "$B" && qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$STAGE$PREFIX/include/luajit-2.1" \
  -o lpeg.so "$S"/lpcap.c "$S"/lpcode.c "$S"/lpcset.c "$S"/lpprint.c "$S"/lptree.c "$S"/lpvm.c && \
  for f in lpcap lpcode lpcset lpprint lptree lpvm; do qcc -V$QCC_TARGET -O2 -fPIC -D_QNX_SOURCE -I"$STAGE$PREFIX/include/luajit-2.1" \
    -c -o "$f.o" "$S/$f.c" || return 1; done && \
  rm -f liblpeg.a && "$QNX_HOST/usr/bin/$QNX_TRIPLE-ar" rcs liblpeg.a lpcap.o lpcode.o lpcset.o lpprint.o lptree.o lpvm.o; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/lib/lua/5.1" "$STAGE$PREFIX/share/lua/5.1" && install -m755 "$B/lpeg.so" "$STAGE$PREFIX/lib/lua/5.1/" && \
  install -m644 "$B/liblpeg.a" "$STAGE$PREFIX/lib/" && \
  install -m644 "$S/re.lua" "$STAGE$PREFIX/share/lua/5.1/"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- LPeg, parsing expression grammars for Lua, built for LuaJIT (the Lua 5.1 ABI Neovim uses). Installs lib/lua/5.1/lpeg.so. Version as in the QNX Everywhere 8.0.4r1 image. Also lib/liblpeg.a, for programs that link LPeg in rather than load it: Neovim does (as its own bundled-dependency build does). Linked against the module lpeg.so instead, which has no soname, the program recorded the build host's path to it as DT_NEEDED and could not load on the target.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH lua-lpeg`.

### 5. What it installs

3 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/lua-lpeg` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa lua-lpeg
ls dists/8.0/*/ | grep '^lua-lpeg-'
```

This writes `lua-lpeg-<ver>-r0.tar.gz` (run time) and `lua-lpeg-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add lua-lpeg
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/lua-lpeg-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/lua-lpeg-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_lua_lpeg`](../../tests/src/) links lua-lpeg the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-lua-lpeg`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-lua-lpeg
/usr/local/bin/qnx-port-tests/test_lua_lpeg
```

```
== lua-lpeg ==
  ok   ...
PASS lua-lpeg
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_lua_lpeg` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_lua_lpeg`](../../tests/src/) -- a program linked against lua-lpeg the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - luajit interpreter found

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lua-lpeg 1.1.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/lua-lpeg-1.1.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lua-lpeg 1.1.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/lua-lpeg-1.1.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lua-lpeg 1.1.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/lua-lpeg-1.1.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lua-lpeg 1.1.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/lua-lpeg-1.1.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.1.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add lua-lpeg=<version>`), sources in `ported/lua-lpeg/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates lua-lpeg`; port one with `scripts/port-update lua-lpeg`.

<details><summary>recipe.sh</summary>

```bash
# LPeg, parsing expression grammars for Lua, built for LuaJIT (the Lua 5.1
# ABI Neovim uses). Installs lib/lua/5.1/lpeg.so. Version as in the QNX
# Everywhere 8.0.4r1 image.
# Also lib/liblpeg.a, for programs that link LPeg in rather than load it:
# Neovim does (as its own bundled-dependency build does). Linked against the
# module lpeg.so instead, which has no soname, the program recorded the
# build host's path to it as DT_NEEDED and could not load on the target.
VERSION=1.1.0
TARBALL=https://www.inf.puc-rio.br/~roberto/lpeg/lpeg-1.1.0.tar.gz
SHA256=4b155d67d2246c1ffa7ad7bc466c1ea899bbc40fef0257cc9c03cecbaed4352a
DEPS="luajit"
BUILDSYS=custom
do_build() { cd "$B" && qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$STAGE$PREFIX/include/luajit-2.1" \
  -o lpeg.so "$S"/lpcap.c "$S"/lpcode.c "$S"/lpcset.c "$S"/lpprint.c "$S"/lptree.c "$S"/lpvm.c && \
  for f in lpcap lpcode lpcset lpprint lptree lpvm; do qcc -V$QCC_TARGET -O2 -fPIC -D_QNX_SOURCE -I"$STAGE$PREFIX/include/luajit-2.1" \
    -c -o "$f.o" "$S/$f.c" || return 1; done && \
  rm -f liblpeg.a && "$QNX_HOST/usr/bin/$QNX_TRIPLE-ar" rcs liblpeg.a lpcap.o lpcode.o lpcset.o lpprint.o lptree.o lpvm.o; }
do_install() { install -d "$STAGE$PREFIX/lib/lua/5.1" "$STAGE$PREFIX/share/lua/5.1" && install -m755 "$B/lpeg.so" "$STAGE$PREFIX/lib/lua/5.1/" && \
  install -m644 "$B/liblpeg.a" "$STAGE$PREFIX/lib/" && \
  install -m644 "$S/re.lua" "$STAGE$PREFIX/share/lua/5.1/"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
