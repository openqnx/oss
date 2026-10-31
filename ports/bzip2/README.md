# bzip2 1.0.8

> Block-sorting file compressor and libbz2 library

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

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add bzip2
apk add bzip2-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-bzip2 && /usr/local/bin/qnx-port-tests/test_bzip2    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install bzip2`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa bzip2
ls dists/8.0/*/ | grep '^bzip2-'
```

This writes `bzip2-<ver>-r0.tar.gz` (run time) and `bzip2-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add bzip2
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/bzip2-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/bzip2-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_bzip2`](../../tests/src/) links bzip2 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-bzip2`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-bzip2
/usr/local/bin/qnx-port-tests/test_bzip2
```

```
== bzip2 ==
  ok   ...
PASS bzip2
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_bzip2` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
   Built again against an *empty* staging tree (`scripts/verify-standalone`): aarch64le passed, x86_64 passed -- it cannot have picked anything up from other components.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_bzip2`](../../tests/src/) -- a program linked against bzip2 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - version string
   - allocate work buffers
   - BZ2_bzBuffToBuffCompress
   - compressed output is smaller
   - BZ2_bzBuffToBuffDecompress
   - round trip is byte-identical

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | bzip2 1.0.8-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/bzip2-1.0.8-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | bzip2 1.0.8-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/bzip2-1.0.8-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.0.8**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add bzip2=<version>`), sources in `ported/bzip2/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates bzip2`; port one with `scripts/port-update bzip2`.

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
