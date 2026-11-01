# fast-float 8.0.2

> Fast and exact implementation of from_chars for numbers

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/fastfloat/fast_float.git (tag `v8.0.2`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_fast_float`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `fast-float-dev` -- header-only: there is no run-time package, the -dev package is the whole component (headers, pkg-config/CMake files).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`fast-float-dev`](https://openqnx.github.io/oss/package/x86_64/fast-float-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/fast-float-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add fast-float-dev` (dependencies come along); one version: `apk add fast-float-dev=<version>` | `apk del fast-float-dev` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install fast-float-dev` (dependencies first) | `oss-get remove fast-float-dev` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-fast-float && /usr/local/bin/qnx-port-tests/test_fast_float    # its on-target test, from tests-8.0-<arch>
apk del fast-float-dev qnx-test-fast-float
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/fast-float-dev.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/fast-float-dev.list
rm var/lib/manual/fast-float-dev.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild fast-float` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v8.0.2 --recurse-submodules --shallow-submodules \
    https://github.com/fastfloat/fast_float.git src/fast-float
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/fast-float -B build/$QNX_ARCH/fast-float -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DFASTFLOAT_TEST=OFF \
      -DFASTFLOAT_INSTALL=ON
cmake --build build/$QNX_ARCH/fast-float -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/fast-float
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- fast_float: header-only C++ implementation of from_chars for floating-point and integer types, several times faster than strtod (Eisel-Lemire algorithm). Build dependency of folly.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH fast-float`.

### 5. What it installs

12 files under `/usr/local`, 9 headers.

The complete list is `stage/<arch>/.manifest/fast-float` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa fast-float
ls dists/8.0/*/ | grep '^fast-float-'
```

This writes `fast-float-<ver>-r0.tar.gz` (run time) and `fast-float-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add fast-float
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/fast-float-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/fast-float-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_fast_float`](../../tests/src/) links fast-float the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-fast-float`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-fast-float
/usr/local/bin/qnx-port-tests/test_fast_float
```

```
== fast-float ==
  ok   ...
PASS fast-float
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_fast_float` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_fast_float`](../../tests/src/) -- a program linked against fast-float the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - pi to 17 digits is M_PI exactly
   - 0.1 -> 0x1.999999999999ap-4
   - 2^53+1 rounds to even (2^53)
   - 2^53+3 rounds to even (2^53+4)
   - smallest subnormal
   - largest subnormal
   - DBL_MAX
   - -0 keeps its sign
   - 1e400 overflows to infinity
   - float: 2^24+1 rounds to 2^24
   - float: FLT_MAX
   - partial parse stops after 12.5e3
   - invalid input: invalid_argument
   - chars_format::fixed ignores the exponent
   - int: INT32_MIN
   - int: 2^31 out of range
   - base 16: ff = 255
   - base 8: 777 = 511
   - 1e23 and 2^56 as the compiler rounds them

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> fast-float`.

## Versions

This recipe builds **8.0.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add fast-float=<version>`), sources in `ported/fast-float/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates fast-float`; port one with `scripts/port-update fast-float`.

<details><summary>recipe.sh</summary>

```bash
# fast_float: header-only C++ implementation of from_chars for floating-point
# and integer types, several times faster than strtod (Eisel-Lemire
# algorithm). Build dependency of folly.
VERSION=8.0.2; GIT=https://github.com/fastfloat/fast_float.git; REF=v8.0.2
CMAKE_ARGS_EXTRA=( -DFASTFLOAT_TEST=OFF -DFASTFLOAT_INSTALL=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
