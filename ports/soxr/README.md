# soxr 0.1.3

> SoX Resampler, high quality one-dimensional sample-rate conversion library

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/chirlu/soxr.git (tag `0.1.3`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_soxr`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `soxr` (programs, shared libraries, run-time data) and `soxr-dev` (headers, pkg-config/CMake files, static libraries; needs `soxr`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`soxr`](https://openqnx.github.io/oss/package/x86_64/soxr.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/soxr.html)) · [`soxr-dev`](https://openqnx.github.io/oss/package/x86_64/soxr-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/soxr-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add soxr` (dependencies come along); one version: `apk add soxr=<version>` | `apk del soxr` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install soxr` (dependencies first) | `oss-get remove soxr` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`soxr-dev` the same way, by its own name (`apk add soxr-dev`, `oss-get install soxr-dev`, by hand with its own file and list `soxr-dev.list`); remove it before `soxr`, which it needs.

```sh
# on the QNX target, as root
apk add soxr soxr-dev
apk add qnx-test-soxr && /usr/local/bin/qnx-port-tests/test_soxr    # its on-target test, from tests-8.0-<arch>-m-z
apk del soxr soxr-dev qnx-test-soxr
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/soxr.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/soxr.list
rm var/lib/manual/soxr.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild soxr` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 0.1.3 --recurse-submodules --shallow-submodules \
    https://github.com/chirlu/soxr.git src/soxr
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/soxr -B build/$QNX_ARCH/soxr -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TESTS=OFF \
      -DWITH_OPENMP=OFF \
      -DBUILD_EXAMPLES=OFF \
      -DWITH_LSR_BINDINGS=ON
cmake --build build/$QNX_ARCH/soxr -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/soxr
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH soxr`.

### 5. What it installs

13 files under `/usr/local`: 6 shared-library entries, 2 headers.

```
/usr/local/lib/libsoxr-lsr.so
/usr/local/lib/libsoxr-lsr.so.0
/usr/local/lib/libsoxr-lsr.so.0.1.9
/usr/local/lib/libsoxr.so
/usr/local/lib/libsoxr.so.0
/usr/local/lib/libsoxr.so.0.1.2
```

The complete list is `stage/<arch>/.manifest/soxr` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa soxr
ls dists/8.0/*/ | grep '^soxr-'
```

This writes `soxr-<ver>-r0.tar.gz` (run time) and `soxr-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add soxr
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/soxr-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/soxr-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_soxr`](../../tests/src/) links soxr the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-soxr`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-soxr
/usr/local/bin/qnx-port-tests/test_soxr
```

```
== soxr ==
  ok   ...
PASS soxr
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_soxr` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_soxr`](../../tests/src/) -- a program linked against soxr the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - soxr_oneshot
   - all input consumed
   - output length is about half the input
   - mean energy per sample is preserved
   - soxr_create (stereo 48k -> 44.1k)
   - soxr_engine names a resampler

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | soxr 0.1.3-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/soxr-0.1.3-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | soxr 0.1.3-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/soxr-0.1.3-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.1.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add soxr=<version>`), sources in `ported/soxr/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates soxr`; port one with `scripts/port-update soxr`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.1.3; GIT=https://github.com/chirlu/soxr.git; REF=0.1.3
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTS=OFF -DWITH_OPENMP=OFF
                   -DBUILD_EXAMPLES=OFF -DWITH_LSR_BINDINGS=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
