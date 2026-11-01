# xtl 0.8.2

> Basic tools, containers and algorithms for C++ (header-only, xtensor)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/xtensor-stack/xtl.git (tag `0.8.2`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xtl`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `xtl-dev` -- header-only: there is no run-time package, the -dev package is the whole component (headers, pkg-config/CMake files).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`xtl-dev`](https://openqnx.github.io/oss/package/x86_64/xtl-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/xtl-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add xtl-dev` (dependencies come along); one version: `apk add xtl-dev=<version>` | `apk del xtl-dev` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install xtl-dev` (dependencies first) | `oss-get remove xtl-dev` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-xtl && /usr/local/bin/qnx-port-tests/test_xtl    # its on-target test, from tests-8.0-<arch>-m-z
apk del xtl-dev qnx-test-xtl
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/xtl-dev.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/xtl-dev.list
rm var/lib/manual/xtl-dev.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xtl` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 0.8.2 --recurse-submodules --shallow-submodules \
    https://github.com/xtensor-stack/xtl.git src/xtl
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xtl < ports/xtl/patches/0001-qnx-executable-path.patch
```

**[`0001-qnx-executable-path.patch`](patches/0001-qnx-executable-path.patch)** — executable_path(): read /proc/self/exefile on QNX

> xtl::executable_path() has branches for Linux (/proc/self/exe), Windows,
> macOS, FreeBSD and Solaris and returns an empty string anywhere else, so on
> QNX prefix_path() -- which xeus kernels use to find their share/ data --
> came out as "/". QNX has no /proc/self/exe symlink; the path of the running
> executable is the content of the file /proc/self/exefile.

Touches: `include/xtl/xsystem.hpp` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/xtl -B build/$QNX_ARCH/xtl -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_TESTS=OFF \
      -DDOWNLOAD_GTEST=OFF
cmake --build build/$QNX_ARCH/xtl -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/xtl
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xtl: QuantStack's header-only C++ template library (basic tools, containers, algorithms) used by xtensor and xeus. Header-only: the CMake build installs the headers and the xtlConfig.cmake package files; nothing is compiled.
- The nlohmann_json integration header (xtl/xjson.hpp) is always installed; it needs nlohmann_json only in the consumer, so no DEPS. Tests are off.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xtl`.

### 5. What it installs

34 files under `/usr/local`, 30 headers.

The complete list is `stage/<arch>/.manifest/xtl` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xtl
ls dists/8.0/*/ | grep '^xtl-'
```

This writes `xtl-<ver>-r0.tar.gz` (run time) and `xtl-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xtl
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xtl-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xtl-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xtl`](../../tests/src/) links xtl the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xtl`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xtl
/usr/local/bin/qnx-port-tests/test_xtl
```

```
== xtl ==
  ok   ...
PASS xtl
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xtl` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xtl`](../../tests/src/) -- a program linked against xtl the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - base64(\"foobar\") = Zm9vYmFy
   - base64(\"fo\") = Zm8= (padding)
   - base64 decode Zm9vYg== = foob
   - base64 round trip of all 256 byte values
   - half 1.5 is exact
   - half 1.5 + 2.25 = 3.75
   - half max 65504
   - half 70000 overflows to inf
   - half(0.1) rounds to 0.0999755859375
   - optional 2 * 3 = 6
   - optional + missing is missing
   - missing value_or(-1) = -1
   - 70-bit dynamic bitset with 3 bits set: count 3
   - bit 33 set, bit 34 clear
   - flip(): 67 bits set
   - fixed string append, max_size 16
   - fixed string find(\"port\") = 4
   - span over {2,3,4}: sum 9
   - executable_path() is the absolute path of this program

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> xtl`.

## Versions

This recipe builds **0.8.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xtl=<version>`), sources in `ported/xtl/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xtl`; port one with `scripts/port-update xtl`.

<details><summary>recipe.sh</summary>

```bash
# xtl: QuantStack's header-only C++ template library (basic tools, containers,
# algorithms) used by xtensor and xeus. Header-only: the CMake build installs
# the headers and the xtlConfig.cmake package files; nothing is compiled.
VERSION=0.8.2; GIT=https://github.com/xtensor-stack/xtl.git; REF=0.8.2
BUILDSYS=cmake
# The nlohmann_json integration header (xtl/xjson.hpp) is always installed;
# it needs nlohmann_json only in the consumer, so no DEPS. Tests are off.
CMAKE_ARGS_EXTRA=( -DBUILD_TESTS=OFF -DDOWNLOAD_GTEST=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
