# exiv2 0.28.5

> C++ library and tool for reading and writing Exif, IPTC and XMP metadata

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/Exiv2/exiv2.git (tag `v0.28.5`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) [`expat`](../expat/) [`brotli`](../brotli/) [`libinih`](../libinih/) |
| Run-time dependencies | [`brotli`](../brotli/) [`expat`](../expat/) [`libinih`](../libinih/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_exiv2`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add exiv2
apk add exiv2-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-exiv2 && /usr/local/bin/qnx-port-tests/test_exiv2    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install exiv2`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild exiv2` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (4 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib expat brotli libinih
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.28.5 --recurse-submodules --shallow-submodules \
    https://github.com/Exiv2/exiv2.git src/exiv2
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/exiv2 -B build/$QNX_ARCH/exiv2 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DEXIV2_BUILD_SAMPLES=OFF \
      -DEXIV2_BUILD_UNIT_TESTS=OFF \
      -DEXIV2_BUILD_EXIV2_COMMAND=OFF \
      -DEXIV2_ENABLE_NLS=OFF \
      -DEXIV2_ENABLE_BMFF=ON
cmake --build build/$QNX_ARCH/exiv2 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/exiv2
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH exiv2`.

### 5. What it installs

56 files under `/usr/local`: 3 shared-library entries, 47 headers.

```
/usr/local/lib/libexiv2.so
/usr/local/lib/libexiv2.so.0.28.5
/usr/local/lib/libexiv2.so.28
```

The complete list is `stage/<arch>/.manifest/exiv2` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa exiv2
ls dists/8.0/*/ | grep '^exiv2-'
```

This writes `exiv2-<ver>-r0.tar.gz` (run time) and `exiv2-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add exiv2
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/exiv2-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/exiv2-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_exiv2`](../../tests/src/) links exiv2 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-exiv2`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-exiv2
/usr/local/bin/qnx-port-tests/test_exiv2
```

```
== exiv2 ==
  ok   ...
PASS exiv2
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_exiv2` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_exiv2`](../../tests/src/) -- a program linked against exiv2 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - ImageFactory::open on an in-memory JPEG
   - mime type is image/jpeg
   - metadata made the file bigger
   - Exif.Image.Make survives
   - a numeric EXIF tag survives
   - IPTC caption survives
   - XMP title survives (expat is wired up)
   - no exception while round-tripping metadata

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | exiv2 0.28.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/exiv2-0.28.5-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | exiv2 0.28.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/exiv2-0.28.5-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.28.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add exiv2=<version>`), sources in `ported/exiv2/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates exiv2`; port one with `scripts/port-update exiv2`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.28.5; GIT=https://github.com/Exiv2/exiv2.git; REF=v0.28.5
DEPS="zlib expat brotli libinih"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DEXIV2_BUILD_SAMPLES=OFF
                   -DEXIV2_BUILD_UNIT_TESTS=OFF -DEXIV2_BUILD_EXIV2_COMMAND=OFF
                   -DEXIV2_ENABLE_NLS=OFF -DEXIV2_ENABLE_BMFF=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
