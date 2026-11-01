# giflib 5.2.2

> Library and tools for reading and writing GIF images

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/giflib/giflib-5.2.2.tar.gz |
| SHA-256 | `be7ffbd057cadebe2aa144542fd90c6838c6a083b5e8a9048b8ee3b66b29d5fb` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Built alone (empty stage) | aarch64le ✅ x86_64 ✅ |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_giflib`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `giflib` (programs, shared libraries, run-time data) and `giflib-dev` (headers, pkg-config/CMake files, static libraries; needs `giflib`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`giflib`](https://openqnx.github.io/oss/package/x86_64/giflib.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/giflib.html)) · [`giflib-dev`](https://openqnx.github.io/oss/package/x86_64/giflib-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/giflib-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add giflib` (dependencies come along); one version: `apk add giflib=<version>` | `apk del giflib` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install giflib` (dependencies first) | `oss-get remove giflib` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`giflib-dev` the same way, by its own name (`apk add giflib-dev`, `oss-get install giflib-dev`, by hand with its own file and list `giflib-dev.list`); remove it before `giflib`, which it needs.

```sh
# on the QNX target, as root
apk add giflib giflib-dev
apk add qnx-test-giflib && /usr/local/bin/qnx-port-tests/test_giflib    # its on-target test, from tests-8.0-<arch>
apk del giflib giflib-dev qnx-test-giflib
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/giflib.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/giflib.list
rm var/lib/manual/giflib.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild giflib` performs exactly the same steps (2–5) in one command.

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
URL=https://downloads.sourceforge.net/giflib/giflib-5.2.2.tar.gz
mkdir -p dist src/giflib
curl -fL -o dist/$(basename $URL) $URL
echo "be7ffbd057cadebe2aa144542fd90c6838c6a083b5e8a9048b8ee3b66b29d5fb  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/giflib --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/giflib; B=build/$QNX_ARCH/giflib; mkdir -p $B
```

```bash
do_configure() { :; }
```

```bash
do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  for f in dgif_lib egif_lib gifalloc gif_err gif_font gif_hash openbsd-reallocarray; do
    $CC -O2 -fPIC -D_QNX_SOURCE -I"$S" -c "$S/$f.c" -o "$f.o" || return 1
  done
  $CC -shared -Wl,-soname,libgif.so.7 -o libgif.so.7.2.0 ./*.o || return 1
  ln -sf libgif.so.7.2.0 libgif.so.7; ln -sf libgif.so.7.2.0 libgif.so
}
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include"
  install -m644 "$B/libgif.so.7.2.0" "$STAGE$PREFIX/lib/"
  cp -a "$B/libgif.so.7" "$B/libgif.so" "$STAGE$PREFIX/lib/"
  install -m644 "$S/gif_lib.h" "$STAGE$PREFIX/include/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- giflib ships only a Makefile that assumes the host toolchain.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH giflib`.

### 5. What it installs

4 files under `/usr/local`: 3 shared-library entries, 1 headers.

```
/usr/local/lib/libgif.so
/usr/local/lib/libgif.so.7
/usr/local/lib/libgif.so.7.2.0
```

The complete list is `stage/<arch>/.manifest/giflib` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa giflib
ls dists/8.0/*/ | grep '^giflib-'
```

This writes `giflib-<ver>-r0.tar.gz` (run time) and `giflib-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add giflib
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/giflib-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/giflib-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_giflib`](../../tests/src/) links giflib the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-giflib`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-giflib
/usr/local/bin/qnx-port-tests/test_giflib
```

```
== giflib ==
  ok   ...
PASS giflib
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_giflib` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
   Built again against an *empty* staging tree (`scripts/verify-standalone`): aarch64le passed, x86_64 passed -- it cannot have picked anything up from other components.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_giflib`](../../tests/src/) -- a program linked against giflib the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - create temp file
   - EGifOpenFileName
   - GifMakeMapObject
   - EGifPutScreenDesc
   - EGifPutImageDesc
   - EGifCloseFile
   - DGifOpenFileName
   - DGifSlurp
   - screen dimensions match
   - one image in the file
   - raster survives the round trip

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | giflib 5.2.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/giflib-5.2.2-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | giflib 5.2.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/giflib-5.2.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **5.2.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add giflib=<version>`), sources in `ported/giflib/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates giflib`; port one with `scripts/port-update giflib`.

<details><summary>recipe.sh</summary>

```bash
VERSION=5.2.2
TARBALL=https://downloads.sourceforge.net/giflib/giflib-5.2.2.tar.gz
SHA256=be7ffbd057cadebe2aa144542fd90c6838c6a083b5e8a9048b8ee3b66b29d5fb
BUILDSYS=custom
# giflib ships only a Makefile that assumes the host toolchain.
do_configure() { :; }
do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  for f in dgif_lib egif_lib gifalloc gif_err gif_font gif_hash openbsd-reallocarray; do
    $CC -O2 -fPIC -D_QNX_SOURCE -I"$S" -c "$S/$f.c" -o "$f.o" || return 1
  done
  $CC -shared -Wl,-soname,libgif.so.7 -o libgif.so.7.2.0 ./*.o || return 1
  ln -sf libgif.so.7.2.0 libgif.so.7; ln -sf libgif.so.7.2.0 libgif.so
}
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include"
  install -m644 "$B/libgif.so.7.2.0" "$STAGE$PREFIX/lib/"
  cp -a "$B/libgif.so.7" "$B/libgif.so" "$STAGE$PREFIX/lib/"
  install -m644 "$S/gif_lib.h" "$STAGE$PREFIX/include/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
