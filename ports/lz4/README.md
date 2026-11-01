# lz4 1.10.0

> Fast lossless compression library and tool (LZ4)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/lz4/lz4.git (tag `v1.10.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_lz4`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `lz4` (programs, shared libraries, run-time data) and `lz4-dev` (headers, pkg-config/CMake files, static libraries; needs `lz4`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`lz4`](https://openqnx.github.io/oss/package/x86_64/lz4.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lz4.html)) · [`lz4-dev`](https://openqnx.github.io/oss/package/x86_64/lz4-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lz4-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add lz4` (dependencies come along); one version: `apk add lz4=<version>` | `apk del lz4` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install lz4` (dependencies first) | `oss-get remove lz4` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`lz4-dev` the same way, by its own name (`apk add lz4-dev`, `oss-get install lz4-dev`, by hand with its own file and list `lz4-dev.list`); remove it before `lz4`, which it needs.

```sh
# on the QNX target, as root
apk add lz4 lz4-dev
apk add qnx-test-lz4 && /usr/local/bin/qnx-port-tests/test_lz4    # its on-target test, from tests-8.0-<arch>
apk del lz4 lz4-dev qnx-test-lz4
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/lz4.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/lz4.list
rm var/lib/manual/lz4.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild lz4` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v1.10.0 --recurse-submodules --shallow-submodules \
    https://github.com/lz4/lz4.git src/lz4
```

The project lives in the `build/cmake/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/lz4/build/cmake -B build/$QNX_ARCH/lz4 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_STATIC_LIBS=ON \
      -DLZ4_BUILD_CLI=OFF \
      -DLZ4_BUILD_LEGACY_LZ4C=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/lz4 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/lz4
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH lz4`.

### 5. What it installs

14 files under `/usr/local`: 3 shared-library entries, 4 headers.

```
/usr/local/lib/liblz4.so
/usr/local/lib/liblz4.so.1
/usr/local/lib/liblz4.so.1.10.0
```

The complete list is `stage/<arch>/.manifest/lz4` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa lz4
ls dists/8.0/*/ | grep '^lz4-'
```

This writes `lz4-<ver>-r0.tar.gz` (run time) and `lz4-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add lz4
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/lz4-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/lz4-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_lz4`](../../tests/src/) links lz4 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-lz4`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-lz4
/usr/local/bin/qnx-port-tests/test_lz4
```

```
== lz4 ==
  ok   ...
PASS lz4
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_lz4` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_lz4`](../../tests/src/) -- a program linked against lz4 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - LZ4_compress_default
   - decompressed size matches
   - round trip matches

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lz4 1.10.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/lz4-1.10.0-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lz4 1.10.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/lz4-1.10.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.10.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add lz4=<version>`), sources in `ported/lz4/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates lz4`; port one with `scripts/port-update lz4`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.10.0; GIT=https://github.com/lz4/lz4.git; REF=v1.10.0
SUBDIR=build/cmake
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=ON
                   -DLZ4_BUILD_CLI=OFF -DLZ4_BUILD_LEGACY_LZ4C=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
