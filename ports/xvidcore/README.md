# xvidcore 1.3.7

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://downloads.xvid.com/downloads/xvidcore-1.3.7.tar.gz |
| SHA-256 | `abbdcbd39555691dd1c9b4d08f0a031376a3b211652c0d8b3b8aa9be1303ce2d` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xvidcore`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add xvidcore
apk add xvidcore-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-xvidcore && /usr/local/bin/qnx-port-tests/test_xvidcore    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install xvidcore`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xvidcore` performs exactly the same steps (2–5) in one command.

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
URL=https://downloads.xvid.com/downloads/xvidcore-1.3.7.tar.gz
mkdir -p dist src/xvidcore
curl -fL -o dist/$(basename $URL) $URL
echo "abbdcbd39555691dd1c9b4d08f0a031376a3b211652c0d8b3b8aa9be1303ce2d  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xvidcore --strip-components=1
```

The project lives in the `build/generic/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/xvidcore/build/generic; B=build/$QNX_ARCH/xvidcore; mkdir -p $B
```

```bash
do_configure() {
  cd "$S" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
    ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" --disable-assembly
}
```

```bash
do_build()   { make -C "$S" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$S" install DESTDIR="$STAGE"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xvid ships its Makefile in the source directory and configure only writes platform.inc beside it, so there is nothing to build out-of-tree. Configure and build in place, the way its own bootstrap.sh expects.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xvidcore`.

### 5. What it installs

5 files under `/usr/local`: 3 shared-library entries, 1 headers.

```
/usr/local/lib/libxvidcore.so
/usr/local/lib/libxvidcore.so.4
/usr/local/lib/libxvidcore.so.4.3
```

The complete list is `stage/<arch>/.manifest/xvidcore` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xvidcore
ls dists/8.0/*/ | grep '^xvidcore-'
```

This writes `xvidcore-<ver>-r0.tar.gz` (run time) and `xvidcore-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xvidcore
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xvidcore-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xvidcore-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xvidcore`](../../tests/src/) links xvidcore the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xvidcore`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xvidcore
/usr/local/bin/qnx-port-tests/test_xvidcore
```

```
== xvidcore ==
  ok   ...
PASS xvidcore
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xvidcore` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xvidcore`](../../tests/src/) -- a program linked against xvidcore the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - xvid_global(XVID_GBL_INIT)
   - xvid_global(XVID_GBL_INFO)
   - reports version 1.x
   - detected at least one cpu

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.3.7**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xvidcore=<version>`), sources in `ported/xvidcore/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xvidcore`; port one with `scripts/port-update xvidcore`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.3.7
TARBALL=https://downloads.xvid.com/downloads/xvidcore-1.3.7.tar.gz
SHA256=abbdcbd39555691dd1c9b4d08f0a031376a3b211652c0d8b3b8aa9be1303ce2d
SUBDIR=build/generic
BUILDSYS=custom
# xvid ships its Makefile in the source directory and configure only writes
# platform.inc beside it, so there is nothing to build out-of-tree. Configure and
# build in place, the way its own bootstrap.sh expects.
do_configure() {
  cd "$S" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
    ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" --disable-assembly
}
do_build()   { make -C "$S" -j"$(nproc)"; }
do_install() { make -C "$S" install DESTDIR="$STAGE"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
