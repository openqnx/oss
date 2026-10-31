# sleef 3.8-git-20250214

> SIMD library of vectorized elementary math functions and DFT

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/shibatch/sleef.git |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_sleef`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `sleef` (programs, shared libraries, run-time data) and `sleef-dev` (headers, pkg-config/CMake files, static libraries; needs `sleef`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`sleef`](https://openqnx.github.io/oss/package/x86_64/sleef.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/sleef.html)) · [`sleef-dev`](https://openqnx.github.io/oss/package/x86_64/sleef-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/sleef-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add sleef` (dependencies come along); one version: `apk add sleef=<version>` | `apk del sleef` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install sleef` (dependencies first) | `oss-get remove sleef` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`sleef-dev` the same way, by its own name (`apk add sleef-dev`, `oss-get install sleef-dev`, by hand with its own file and list `sleef-dev.list`); remove it before `sleef`, which it needs.

```sh
# on the QNX target, as root
apk add sleef sleef-dev
apk add qnx-test-sleef && /usr/local/bin/qnx-port-tests/test_sleef    # its on-target test
apk del sleef sleef-dev qnx-test-sleef
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/sleef.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/sleef.list
rm var/lib/manual/sleef.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild sleef` performs exactly the same steps (2–5) in one command.

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
    https://github.com/shibatch/sleef.git src/sleef
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/sleef < ports/sleef/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add QNX support.

> Imported from qnx-ports/sleef (b8abbd56df03...qnx_master); original authorship kept.
>
>
> ---
>  Configure.cmake        | 16 ++++++++++++----
>  src/dft/CMakeLists.txt |  4 ++--
>  2 files changed, 14 insertions(+), 6 deletions(-)

Touches: `Configure.cmake`, `src/dft/CMakeLists.txt` · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/sleef; B=build/$QNX_ARCH/sleef; mkdir -p $B
```

```bash
do_configure() {
  local nat="$B/native"; mkdir -p "$nat"
  env -u CC -u CXX cmake -S "$S" -B "$nat" -G Ninja -DCMAKE_BUILD_TYPE=Release -DSLEEF_BUILD_TESTS=OFF \
      -DSLEEF_BUILD_DFT=OFF -DSLEEF_BUILD_QUAD=OFF -DSLEEF_BUILD_GNUABI_LIBS=OFF && ninja -C "$nat" || return 1
  cmake -S "$S" -B "$B/target" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$arch.cmake" -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="$PREFIX" -DNATIVE_BUILD_DIR="$nat" -DSLEEF_BUILD_TESTS=OFF -DSLEEF_BUILD_SHARED_LIBS=ON \
      -DSLEEF_BUILD_DFT=OFF -DSLEEF_BUILD_QUAD=OFF -DSLEEF_BUILD_GNUABI_LIBS=OFF -DSLEEF_BUILD_INLINE_HEADERS=OFF
}
```

```bash
do_build() { ninja -C "$B/target"; }
```

```bash
do_install() { DESTDIR="$STAGE" cmake --install "$B/target"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- SLEEF generates parts of itself with helper programs that must run on the build host: a native build of those helpers is made first (do_configure).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH sleef`.

### 5. What it installs

9 files under `/usr/local`: 3 shared-library entries, 1 headers.

```
/usr/local/lib/libsleef.so
/usr/local/lib/libsleef.so.3
/usr/local/lib/libsleef.so.3.9.0
```

The complete list is `stage/<arch>/.manifest/sleef` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa sleef
ls dists/8.0/*/ | grep '^sleef-'
```

This writes `sleef-<ver>-r0.tar.gz` (run time) and `sleef-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add sleef
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/sleef-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/sleef-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_sleef`](../../tests/src/) links sleef the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-sleef`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-sleef
/usr/local/bin/qnx-port-tests/test_sleef
```

```
== sleef ==
  ok   ...
PASS sleef
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_sleef` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_sleef`](../../tests/src/) -- a program linked against sleef the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - sin(1)
   - cos(1)
   - sin(1e6) (large-argument reduction)
   - exp(1) = e
   - log(10)
   - pow(2, 0.5) = sqrt 2
   - pow(3, 4) = 81 exactly
   - 4*atan2(1,1) = pi
   - sinpi(1/6) = 0.5
   - tgamma(5) = 24
   - erf(0.5)
   - cbrt(27) = 3
   - sincos(0.5)
   - special values: log(-1)=NaN, exp overflow/underflow
   - float sinf/expf
   - Sleef_sind2_u10 (2 lanes)
   - Sleef_expd2_u10 (2 lanes)
   - Sleef_logf4_u10 (4 lanes)

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | sleef 3.8_git20250214-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/sleef-3.8_git20250214-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | sleef 3.8_git20250214-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/sleef-3.8_git20250214-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | sleef 3.8_git20250214-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/sleef-3.8_git20250214-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | sleef 3.8_git20250214-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/sleef-3.8_git20250214-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.8-git-20250214**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add sleef=<version>`), sources in `ported/sleef/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates sleef`; port one with `scripts/port-update sleef`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.8-git-20250214; GIT=https://github.com/shibatch/sleef.git; COMMIT=b8abbd56df03eb7cc65b8025495b955ce5336973
# SLEEF generates parts of itself with helper programs that must run on the
# build host: a native build of those helpers is made first (do_configure).
do_configure() {
  local nat="$B/native"; mkdir -p "$nat"
  env -u CC -u CXX cmake -S "$S" -B "$nat" -G Ninja -DCMAKE_BUILD_TYPE=Release -DSLEEF_BUILD_TESTS=OFF \
      -DSLEEF_BUILD_DFT=OFF -DSLEEF_BUILD_QUAD=OFF -DSLEEF_BUILD_GNUABI_LIBS=OFF && ninja -C "$nat" || return 1
  cmake -S "$S" -B "$B/target" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$arch.cmake" -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="$PREFIX" -DNATIVE_BUILD_DIR="$nat" -DSLEEF_BUILD_TESTS=OFF -DSLEEF_BUILD_SHARED_LIBS=ON \
      -DSLEEF_BUILD_DFT=OFF -DSLEEF_BUILD_QUAD=OFF -DSLEEF_BUILD_GNUABI_LIBS=OFF -DSLEEF_BUILD_INLINE_HEADERS=OFF
}
do_build() { ninja -C "$B/target"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B/target"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
