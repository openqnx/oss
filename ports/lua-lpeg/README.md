# lua-lpeg 1.1.0

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
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add lua-lpeg
apk add lua-lpeg-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install lua-lpeg`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lua-lpeg 1.1.0-r0 | ✅ PASS | 2026-10-30 |

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
