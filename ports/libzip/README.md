# libzip 1.11.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/nih-at/libzip.git (tag `v1.11.1`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) [`bzip2`](../bzip2/) [`xz`](../xz/) [`zstd`](../zstd/) [`openssl`](../openssl/) |
| Run-time dependencies | [`bzip2`](../bzip2/) [`openssl`](../openssl/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libzip`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libzip
apk add libzip-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libzip && /usr/local/bin/qnx-port-tests/test_libzip    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libzip`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libzip` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (9 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib bzip2 xz zstd openssl
```

<details><summary>Full build chain (9)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.11.1 --recurse-submodules --shallow-submodules \
    https://github.com/nih-at/libzip.git src/libzip
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libzip -B build/$QNX_ARCH/libzip -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_DOC=OFF \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_REGRESS=OFF \
      -DBUILD_OSSFUZZ=OFF \
      -DBUILD_TOOLS=ON \
      -DENABLE_GNUTLS=OFF \
      -DENABLE_MBEDTLS=OFF \
      -DENABLE_OPENSSL=ON
cmake --build build/$QNX_ARCH/libzip -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libzip
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libzip`.

### 5. What it installs

16 files under `/usr/local`: 3 shared-library entries, 3 programs, 2 headers.

```
/usr/local/lib/libzip.so
/usr/local/lib/libzip.so.5
/usr/local/lib/libzip.so.5.5
/usr/local/bin/zipcmp
/usr/local/bin/zipmerge
/usr/local/bin/ziptool
```

The complete list is `stage/<arch>/.manifest/libzip` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libzip
ls dists/8.0/*/ | grep '^libzip-'
```

This writes `libzip-<ver>-r0.tar.gz` (run time) and `libzip-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libzip
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libzip-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libzip-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libzip`](../../tests/src/) links libzip the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libzip`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libzip
/usr/local/bin/qnx-port-tests/test_libzip
```

```
== libzip ==
  ok   ...
PASS libzip
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libzip` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libzip`](../../tests/src/) -- a program linked against libzip the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - create archive
   - add entry
   - write (zstd)
   - read back

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libzip 1.11.1-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libzip 1.11.1-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.11.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libzip=<version>`), sources in `ported/libzip/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libzip`; port one with `scripts/port-update libzip`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.11.1; GIT=https://github.com/nih-at/libzip.git; REF=v1.11.1
DEPS="zlib bzip2 xz zstd openssl"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_DOC=OFF -DBUILD_EXAMPLES=OFF -DBUILD_REGRESS=OFF
                   -DBUILD_OSSFUZZ=OFF -DBUILD_TOOLS=ON -DENABLE_GNUTLS=OFF -DENABLE_MBEDTLS=OFF -DENABLE_OPENSSL=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
