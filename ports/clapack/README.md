# clapack 3.2.1

> LAPACK linear algebra routines translated to C with f2c

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/hunter-packages/clapack.git |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_clapack`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `clapack` (programs, shared libraries, run-time data) and `clapack-dev` (headers, pkg-config/CMake files, static libraries; needs `clapack`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`clapack`](https://openqnx.github.io/oss/package/x86_64/clapack.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/clapack.html)) · [`clapack-dev`](https://openqnx.github.io/oss/package/x86_64/clapack-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/clapack-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add clapack` (dependencies come along); one version: `apk add clapack=<version>` | `apk del clapack` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install clapack` (dependencies first) | `oss-get remove clapack` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`clapack-dev` the same way, by its own name (`apk add clapack-dev`, `oss-get install clapack-dev`, by hand with its own file and list `clapack-dev.list`); remove it before `clapack`, which it needs.

```sh
# on the QNX target, as root
apk add clapack clapack-dev
apk add qnx-test-clapack && /usr/local/bin/qnx-port-tests/test_clapack    # its on-target test, from tests-8.0-<arch>
apk del clapack clapack-dev qnx-test-clapack
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/clapack.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/clapack.list
rm var/lib/manual/clapack.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild clapack` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/hunter-packages/clapack.git src/clapack
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/clapack < ports/clapack/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — ADDED: From GL, support for corss compiling for QNX.

> Generation of test .sh scripts for automating binaries. MODIFIED: Added
>  support for custom /sh paths for test .sh generation
>
> Imported from qnx-ports/clapack (master...qnx_v3.2.1); original authorship kept.
>
>
> ---
>  CMakeLists.txt                |  2 +-
>  F2CLIBS/libf2c/CMakeLists.txt | 21 ++++++++++++++++-----
>  TESTING/CMakeLists.txt        | 34 ++++++++++++++++++++++++++++------
>  TESTING/MATGEN/CMakeLists.txt |  3 +++
>  TESTING/run_test.sh.in        |  5 +++++
>  5 files changed, 53 insertions(+), 12 deletions(-)
>  create mode 100644 TESTING/run_test.sh.in

Touches: `CMakeLists.txt`, `F2CLIBS/libf2c/CMakeLists.txt`, `TESTING/CMakeLists.txt`, `TESTING/MATGEN/CMakeLists.txt`, `TESTING/run_test.sh.in` · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/clapack; B=build/$QNX_ARCH/clapack; mkdir -p $B
```

```bash
do_configure() {
  gcc -DNO_FPINIT -o "$B/arithchk" "$S/F2CLIBS/libf2c/arithchk.c" -lm || return 1
  ARITHCHK="$B/arithchk" cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=ON
}
```

```bash
do_build() { cmake --build "$B" -j"$(nproc)"; }
```

```bash
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- CLAPACK (LAPACK translated to C with f2c), in the CMake packaging of hunter-packages/clapack -- the tree the qnx-ports port is based on. Prefer OpenBLAS's LAPACK for speed; this is for code that links CLAPACK directly.
- libf2c's arith.h is generated by running arithchk, which describes the floating-point model of the machine it runs on; both QNX targets match the x86_64 build host (IEEE 754, little endian, 64-bit), so it is built and run natively -- as qnx-ports does.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH clapack`.

### 5. What it installs

9 files under `/usr/local`: 3 shared-library entries, 3 headers.

```
/usr/local/lib/libblas.so
/usr/local/lib/libf2c.so
/usr/local/lib/liblapack.so
```

The complete list is `stage/<arch>/.manifest/clapack` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa clapack
ls dists/8.0/*/ | grep '^clapack-'
```

This writes `clapack-<ver>-r0.tar.gz` (run time) and `clapack-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add clapack
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/clapack-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/clapack-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_clapack`](../../tests/src/) links clapack the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-clapack`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-clapack
/usr/local/bin/qnx-port-tests/test_clapack
```

```
== clapack ==
  ok   ...
PASS clapack
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_clapack` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_clapack`](../../tests/src/) -- a program linked against clapack the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - DGEMM: [[58,64],[139,154]]
   - DDOT = 12
   - DGESV solves to (2, 3, -1)
   - DGETRF determinant = -306
   - DPOTRF Cholesky factor
   - DSYEV eigenvalues
   - DSYEV eigenvector for lambda = 2
   - invalid N reported through INFO (XERBLA path)

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | clapack 3.2.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/clapack-3.2.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | clapack 3.2.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/clapack-3.2.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | clapack 3.2.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/clapack-3.2.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | clapack 3.2.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/clapack-3.2.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.2.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add clapack=<version>`), sources in `ported/clapack/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates clapack`; port one with `scripts/port-update clapack`.

<details><summary>recipe.sh</summary>

```bash
# CLAPACK (LAPACK translated to C with f2c), in the CMake packaging of
# hunter-packages/clapack -- the tree the qnx-ports port is based on. Prefer
# OpenBLAS's LAPACK for speed; this is for code that links CLAPACK directly.
VERSION=3.2.1; GIT=https://github.com/hunter-packages/clapack.git; COMMIT=3cb7c7c0fccf2480c292875c2d1f099deedd1c41
# libf2c's arith.h is generated by running arithchk, which describes the
# floating-point model of the machine it runs on; both QNX targets match the
# x86_64 build host (IEEE 754, little endian, 64-bit), so it is built and run
# natively -- as qnx-ports does.
do_configure() {
  gcc -DNO_FPINIT -o "$B/arithchk" "$S/F2CLIBS/libf2c/arithchk.c" -lm || return 1
  ARITHCHK="$B/arithchk" cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=ON
}
do_build() { cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
