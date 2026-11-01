# zimg 3.0.5

> Library for image scaling, colorspace conversion and dithering

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/sekrit-twc/zimg.git (tag `release-3.0.5`) |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_zimg`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `zimg` (programs, shared libraries, run-time data) and `zimg-dev` (headers, pkg-config/CMake files, static libraries; needs `zimg`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`zimg`](https://openqnx.github.io/oss/package/x86_64/zimg.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/zimg.html)) · [`zimg-dev`](https://openqnx.github.io/oss/package/x86_64/zimg-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/zimg-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add zimg` (dependencies come along); one version: `apk add zimg=<version>` | `apk del zimg` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install zimg` (dependencies first) | `oss-get remove zimg` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`zimg-dev` the same way, by its own name (`apk add zimg-dev`, `oss-get install zimg-dev`, by hand with its own file and list `zimg-dev.list`); remove it before `zimg`, which it needs.

```sh
# on the QNX target, as root
apk add zimg zimg-dev
apk add qnx-test-zimg && /usr/local/bin/qnx-port-tests/test_zimg    # its on-target test, from tests-8.0-<arch>-m-z
apk del zimg zimg-dev qnx-test-zimg
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/zimg.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/zimg.list
rm var/lib/manual/zimg.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild zimg` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b release-3.0.5 --recurse-submodules --shallow-submodules \
    https://github.com/sekrit-twc/zimg.git src/zimg
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/zimg < ports/zimg/patches/0001-qnx-posix-memalign-malloc-h.patch
```

**[`0001-qnx-posix-memalign-malloc-h.patch`](patches/0001-qnx-posix-memalign-malloc-h.patch)** — zimg: posix_memalign is declared in <malloc.h> on QNX.

> <stdlib.h> alone leaves it undeclared, and every translation unit failed.

Touches: `src/zimg/common/alloc.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/zimg && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/zimg && cd build/$QNX_ARCH/zimg
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/zimg/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH zimg`.

### 5. What it installs

21 files under `/usr/local`: 2 shared-library entries, 2 headers.

```
/usr/local/lib/libzimg.so
/usr/local/lib/libzimg.so.2
```

The complete list is `stage/<arch>/.manifest/zimg` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa zimg
ls dists/8.0/*/ | grep '^zimg-'
```

This writes `zimg-<ver>-r0.tar.gz` (run time) and `zimg-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add zimg
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/zimg-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/zimg-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_zimg`](../../tests/src/) links zimg the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-zimg`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-zimg
/usr/local/bin/qnx-port-tests/test_zimg
```

```
== zimg ==
  ok   ...
PASS zimg
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_zimg` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_zimg`](../../tests/src/) -- a program linked against zimg the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - zimg_filter_graph_build
   - zimg_filter_graph_get_tmp_size
   - aligned buffers allocated
   - zimg_filter_graph_process
   - the destination plane was written
   - flat chroma survives the resize

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | zimg 3.0.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/zimg-3.0.5-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | zimg 3.0.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/zimg-3.0.5-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.0.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add zimg=<version>`), sources in `ported/zimg/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates zimg`; port one with `scripts/port-update zimg`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.0.5; GIT=https://github.com/sekrit-twc/zimg.git; REF=release-3.0.5
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
do_patch() { cd "$SRCROOT" && git submodule update --init --recursive >/dev/null 2>&1; autoreconf -fi; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
