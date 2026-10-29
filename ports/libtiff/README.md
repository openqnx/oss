# libtiff 4.7.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.com/libtiff/libtiff.git (tag `v4.7.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) [`xz`](../xz/) [`zstd`](../zstd/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libwebp`](../libwebp/) |
| Run-time dependencies | [`libjpeg-turbo`](../libjpeg-turbo/) [`libwebp`](../libwebp/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libtiff`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libtiff
apk add libtiff-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libtiff && /usr/local/bin/qnx-port-tests/test_libtiff    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libtiff`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libtiff` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (11 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib xz zstd libjpeg-turbo libwebp
```

<details><summary>Full build chain (11)</summary>

[`gettext`](../gettext/) [`giflib`](../giflib/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v4.7.0 --recurse-submodules --shallow-submodules \
    https://gitlab.com/libtiff/libtiff.git src/libtiff
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libtiff -B build/$QNX_ARCH/libtiff -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -Dtiff-tools=OFF \
      -Dtiff-tests=OFF \
      -Dtiff-docs=OFF \
      -Dtiff-contrib=OFF \
      -Dlerc=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/libtiff -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libtiff
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libtiff`.

### 5. What it installs

16 files under `/usr/local`: 6 shared-library entries, 5 headers.

```
/usr/local/lib/libtiff.so
/usr/local/lib/libtiff.so.6
/usr/local/lib/libtiff.so.6.1.0
/usr/local/lib/libtiffxx.so
/usr/local/lib/libtiffxx.so.6
/usr/local/lib/libtiffxx.so.6.1.0
```

The complete list is `stage/<arch>/.manifest/libtiff` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libtiff
ls dists/8.0/*/ | grep '^libtiff-'
```

This writes `libtiff-<ver>-r0.tar.gz` (run time) and `libtiff-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libtiff
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libtiff-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libtiff-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libtiff`](../../tests/src/) links libtiff the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libtiff`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libtiff
/usr/local/bin/qnx-port-tests/test_libtiff
```

```
== libtiff ==
  ok   ...
PASS libtiff
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libtiff` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libtiff`](../../tests/src/) -- a program linked against libtiff the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - create temp file
   - TIFFOpen for write
   - TIFFWriteScanline for every row
   - TIFFOpen for read
   - dimensions match
   - 3 samples of 8 bits
   - deflate compression survives (zlib is wired up)
   - TIFFReadScanline for every row
   - pixels survive the round trip

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **4.7.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libtiff=<version>`), sources in `ported/libtiff/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libtiff`; port one with `scripts/port-update libtiff`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.7.0; GIT=https://gitlab.com/libtiff/libtiff.git; REF=v4.7.0
DEPS="zlib xz zstd libjpeg-turbo libwebp"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -Dtiff-tools=OFF -Dtiff-tests=OFF
                   -Dtiff-docs=OFF -Dtiff-contrib=OFF -Dlerc=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
