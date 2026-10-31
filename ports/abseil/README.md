# abseil 20240722.0

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/abseil/abseil-cpp.git (tag `20240722.0`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Built alone (empty stage) | aarch64le ✅ x86_64 ✅ |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add abseil
apk add abseil-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install abseil`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild abseil` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 20240722.0 --recurse-submodules --shallow-submodules \
    https://github.com/abseil/abseil-cpp.git src/abseil
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/abseil < ports/abseil/patches/0001-maskedpointer-default-ctor-libcxx.patch
```

**[`0001-maskedpointer-default-ctor-libcxx.patch`](patches/0001-maskedpointer-default-ctor-libcxx.patch)** — Give absl's MaskedPointer a default constructor (libc++).

> std::atomic<T>'s default constructor value-initialises T under libc++, the
> C++ library QNX ships, so std::atomic<MaskedPointer> in absl/flags needs T to
> be default-constructible; libstdc++ does not require it. The constructor
> makes a null pointer with no mask bits, which is what the atomic is always
> assigned before first use.

Touches: `absl/flags/internal/flag.h` · Upstream status: Pending (affects any libc++ build)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/abseil -B build/$QNX_ARCH/abseil -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DABSL_BUILD_TESTING=OFF \
      -DABSL_PROPAGATE_CXX_STD=ON \
      -DCMAKE_CXX_STANDARD=17 \
      -DABSL_ENABLE_INSTALL=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/abseil -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/abseil
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- One abseil for the whole tree.  protobuf and gRPC both vendor abseil and both used to install their copy -- 20240116 (sonames .2401) from protobuf, 20240722 (.2407) from gRPC -- so 86 libabsl_* libraries were staged twice, the unversioned symlinks pointed at whichever installed last, and a gRPC process mapped both.  scripts/check-dupes now catches that.  This is the version gRPC 1.69 pins, and the same one ONNX Runtime vendors (statically, so it does not take part in this). The patch is the libc++ fix gRPC and ORT already needed: MaskedPointer has no default constructor and libc++'s std::atomic<T> requires one.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH abseil`.

### 5. What it installs

756 files under `/usr/local`: 180 shared-library entries, 382 headers.

```
/usr/local/lib/libabsl_bad_any_cast_impl.so
/usr/local/lib/libabsl_bad_any_cast_impl.so.2407.0.0
/usr/local/lib/libabsl_bad_optional_access.so
/usr/local/lib/libabsl_bad_optional_access.so.2407.0.0
/usr/local/lib/libabsl_bad_variant_access.so
/usr/local/lib/libabsl_bad_variant_access.so.2407.0.0
/usr/local/lib/libabsl_base.so
/usr/local/lib/libabsl_base.so.2407.0.0
/usr/local/lib/libabsl_city.so
/usr/local/lib/libabsl_city.so.2407.0.0
/usr/local/lib/libabsl_civil_time.so
/usr/local/lib/libabsl_civil_time.so.2407.0.0
/usr/local/lib/libabsl_cord.so
/usr/local/lib/libabsl_cord.so.2407.0.0
/usr/local/lib/libabsl_cord_internal.so
/usr/local/lib/libabsl_cord_internal.so.2407.0.0
...
```

The complete list is `stage/<arch>/.manifest/abseil` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa abseil
ls dists/8.0/*/ | grep '^abseil-'
```

This writes `abseil-<ver>-r0.tar.gz` (run time) and `abseil-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add abseil
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/abseil-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/abseil-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
   Built again against an *empty* staging tree (`scripts/verify-standalone`): aarch64le passed, x86_64 passed -- it cannot have picked anything up from other components.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | abseil 20240722.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/abseil-20240722.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | abseil 20240722.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/abseil-20240722.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **20240722.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add abseil=<version>`), sources in `ported/abseil/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates abseil`; port one with `scripts/port-update abseil`.

<details><summary>recipe.sh</summary>

```bash
VERSION=20240722.0; GIT=https://github.com/abseil/abseil-cpp.git; REF=20240722.0
BUILDSYS=cmake
# One abseil for the whole tree.  protobuf and gRPC both vendor abseil and both
# used to install their copy -- 20240116 (sonames .2401) from protobuf, 20240722
# (.2407) from gRPC -- so 86 libabsl_* libraries were staged twice, the
# unversioned symlinks pointed at whichever installed last, and a gRPC process
# mapped both.  scripts/check-dupes now catches that.  This is the version gRPC
# 1.69 pins, and the same one ONNX Runtime vendors (statically, so it does not
# take part in this).
#
# The patch is the libc++ fix gRPC and ORT already needed: MaskedPointer has no
# default constructor and libc++'s std::atomic<T> requires one.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DABSL_BUILD_TESTING=OFF
                   -DABSL_PROPAGATE_CXX_STD=ON -DCMAKE_CXX_STANDARD=17
                   -DABSL_ENABLE_INSTALL=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
