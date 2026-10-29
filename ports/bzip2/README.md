# bzip2 1.0.8

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://gitlab.com/bzip2/bzip2.git (tag `bzip2-1.0.8`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Built alone (empty stage) | aarch64le ✅ x86_64 ✅ |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_bzip2`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install bzip2
oss-get install bzip2-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild bzip2` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b bzip2-1.0.8 --recurse-submodules --shallow-submodules \
    https://gitlab.com/bzip2/bzip2.git src/bzip2
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/bzip2; B=build/$QNX_ARCH/bzip2; mkdir -p $B
```

```bash
do_configure() { :; }
```

```bash
do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  local CFLAGS="-O2 -D_FILE_OFFSET_BITS=64 -D_QNX_SOURCE -fPIC -I$S"
  for f in $BZ_SRCS; do $CC $CFLAGS -c "$S/$f" -o "${f%.c}.o" || return 1; done
  $CC -shared -Wl,-soname,libbz2.so.1.0 -o libbz2.so.1.0.8 ./*.o || return 1
  "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs libbz2.a ./*.o || return 1
  ln -sf libbz2.so.1.0.8 libbz2.so.1.0
  ln -sf libbz2.so.1.0.8 libbz2.so.1
  ln -sf libbz2.so.1.0.8 libbz2.so
}
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include"
  install -m644 "$B/libbz2.so.1.0.8" "$B/libbz2.a" "$STAGE$PREFIX/lib/"
  cp -a "$B/libbz2.so.1.0" "$B/libbz2.so.1" "$B/libbz2.so" "$STAGE$PREFIX/lib/"
  install -m644 "$S/bzlib.h" "$STAGE$PREFIX/include/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- bzip2 1.0.8 predates any CMake support and its Makefile hardcodes the host compiler, so drive the compilation directly.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH bzip2`.

### 5. What it installs

6 files under `/usr/local`: 4 shared-library entries, 1 headers.

```
/usr/local/lib/libbz2.so
/usr/local/lib/libbz2.so.1
/usr/local/lib/libbz2.so.1.0
/usr/local/lib/libbz2.so.1.0.8
```

The complete list is `stage/<arch>/.manifest/bzip2` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH bzip2
ls repo/8.0/*/ | grep '^bzip2-'
```

This writes `bzip2-<ver>-r0.tar.gz` (run time) and `bzip2-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install bzip2
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/bzip2-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/bzip2-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_bzip2`](../../tests/src/) links bzip2 the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_bzip2 root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_bzip2'
```

```
== bzip2 ==
  ok   ...
PASS bzip2
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.0.8; GIT=https://gitlab.com/bzip2/bzip2.git; REF=bzip2-1.0.8
BUILDSYS=custom
# bzip2 1.0.8 predates any CMake support and its Makefile hardcodes the host
# compiler, so drive the compilation directly.
BZ_SRCS="blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c"

do_configure() { :; }

do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  local CFLAGS="-O2 -D_FILE_OFFSET_BITS=64 -D_QNX_SOURCE -fPIC -I$S"
  for f in $BZ_SRCS; do $CC $CFLAGS -c "$S/$f" -o "${f%.c}.o" || return 1; done
  $CC -shared -Wl,-soname,libbz2.so.1.0 -o libbz2.so.1.0.8 ./*.o || return 1
  "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs libbz2.a ./*.o || return 1
  ln -sf libbz2.so.1.0.8 libbz2.so.1.0
  ln -sf libbz2.so.1.0.8 libbz2.so.1
  ln -sf libbz2.so.1.0.8 libbz2.so
}

do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include"
  install -m644 "$B/libbz2.so.1.0.8" "$B/libbz2.a" "$STAGE$PREFIX/lib/"
  cp -a "$B/libbz2.so.1.0" "$B/libbz2.so.1" "$B/libbz2.so" "$STAGE$PREFIX/lib/"
  install -m644 "$S/bzlib.h" "$STAGE$PREFIX/include/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
