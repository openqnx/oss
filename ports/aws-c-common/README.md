# aws-c-common 0.12.6

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/awslabs/aws-c-common.git (tag `v0.12.6`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add aws-c-common
apk add aws-c-common-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install aws-c-common`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild aws-c-common` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v0.12.6 --recurse-submodules --shallow-submodules \
    https://github.com/awslabs/aws-c-common.git src/aws-c-common
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/aws-c-common < ports/aws-c-common/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add QNX Support

> Imported from qnx-ports/aws-c-common (v0.12.4...qnx-v0.12.4).
>
> Fix INTERFACE include path
>
> remove debug statement
>
> remove space
> ---
>  CMakeLists.txt | 10 ++++++++--
>  1 file changed, 8 insertions(+), 2 deletions(-)

Touches: `CMakeLists.txt` · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/aws-c-common -B build/$QNX_ARCH/aws-c-common -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" \
      -DAWS_WARNINGS_ARE_ERRORS=OFF
cmake --build build/$QNX_ARCH/aws-c-common -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/aws-c-common
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The release aws-crt-cpp v0.38.5 pins as its submodule, so the whole AWS C runtime is one consistent set.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH aws-c-common`.

### 5. What it installs

101 files under `/usr/local`: 3 shared-library entries, 85 headers.

```
/usr/local/lib/libaws-c-common.so
/usr/local/lib/libaws-c-common.so.1
/usr/local/lib/libaws-c-common.so.1.0.0
```

The complete list is `stage/<arch>/.manifest/aws-c-common` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa aws-c-common
ls dists/8.0/*/ | grep '^aws-c-common-'
```

This writes `aws-c-common-<ver>-r0.tar.gz` (run time) and `aws-c-common-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add aws-c-common
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/aws-c-common-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/aws-c-common-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aws-c-common 0.12.6-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/aws-c-common-0.12.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aws-c-common 0.12.6-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/aws-c-common-0.12.6-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.12.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add aws-c-common=<version>`), sources in `ported/aws-c-common/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates aws-c-common`; port one with `scripts/port-update aws-c-common`.

<details><summary>recipe.sh</summary>

```bash
# The release aws-crt-cpp v0.38.5 pins as its submodule, so the
# whole AWS C runtime is one consistent set.
VERSION=0.12.6; GIT=https://github.com/awslabs/aws-c-common.git; REF=v0.12.6
DEPS=""
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" -DAWS_WARNINGS_ARE_ERRORS=OFF  )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
