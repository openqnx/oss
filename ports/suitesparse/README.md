# suitesparse 7.11.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/DrTimothyAldenDavis/SuiteSparse.git (tag `v7.11.0`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`openblas`](../openblas/) |
| Run-time dependencies | [`openblas`](../openblas/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add suitesparse
apk add suitesparse-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install suitesparse`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild suitesparse` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild openblas
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v7.11.0 --recurse-submodules --shallow-submodules \
    https://github.com/DrTimothyAldenDavis/SuiteSparse.git src/suitesparse
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/suitesparse < ports/suitesparse/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Bump actions/checkout from 4 to 5

> Imported from qnx-ports/SuiteSparse (b35a1f9318f4...qnx-v7.11.0); original authorship kept.
>
>
> Bumps [actions/checkout](https://github.com/actions/checkout) from 4 to 5.
> - [Release notes](https://github.com/actions/checkout/releases)
> - [Changelog](https://github.com/actions/checkout/blob/main/CHANGELOG.md)
> - [Commits](https://github.com/actions/checkout/compare/v4...v5)
>
> ---
> updated-dependencies:
> - dependency-name: actions/checkout
>   dependency-version: '5'
>   dependency-type: direct:production
>   update-type: version-update:semver-major
> ...
>
> Signed-off-by: dependabot[bot] <support@github.com>
> ---
>  .github/workflows/build-arch-emu.yaml       | 2 +-
>  .github/workflows/build-mingw.yaml          | 2 +-
>  .github/workflows/build.yaml                | 2 +-
>  .github/workflows/codeql-analysis.yaml      | 4 ++--
>  .github/workflows/cross-compile.yaml        | 2 +-
>  .github/workflows/macos.yaml                | 2 +-
>  .github/workflows/root-cmakelists-msvc.yaml | 2 +-
>  .github/workflows/root-cmakelists.yaml      | 2 +-
>  8 files changed, 9 insertions(+), 9 deletions(-)

Touches: `.github/workflows/build-arch-emu.yaml`, `.github/workflows/build-mingw.yaml`, `.github/workflows/build.yaml`, `.github/workflows/codeql-analysis.yaml`, `.github/workflows/cross-compile.yaml`, `.github/workflows/macos.yaml` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/suitesparse -B build/$QNX_ARCH/suitesparse -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DSUITESPARSE_USE_FORTRAN=OFF \
      -DSUITESPARSE_USE_CUDA=OFF \
      -DSUITESPARSE_USE_OPENMP=OFF \
      -DBLA_VENDOR=OpenBLAS \
      -DBLAS_LIBRARIES="$STAGE$PREFIX/lib/libopenblas.so" \
      -DLAPACK_LIBRARIES="$STAGE$PREFIX/lib/libopenblas.so" \
      -DSUITESPARSE_DEMOS=OFF \
      -DBUILD_TESTING=OFF \
      -DSUITESPARSE_ENABLE_PROJECTS="suitesparse_config;amd;btf;camd;ccolamd;colamd;cholmod;cxsparse;klu;ldl;umfpack;rbio;spqr"
cmake --build build/$QNX_ARCH/suitesparse -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/suitesparse
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH suitesparse`.

### 5. What it installs

177 files under `/usr/local`: 42 shared-library entries, 16 headers.

```
/usr/local/lib/libamd.so
/usr/local/lib/libamd.so.3
/usr/local/lib/libamd.so.3.3.4
/usr/local/lib/libbtf.so
/usr/local/lib/libbtf.so.2
/usr/local/lib/libbtf.so.2.3.3
/usr/local/lib/libcamd.so
/usr/local/lib/libcamd.so.3
/usr/local/lib/libcamd.so.3.3.5
/usr/local/lib/libccolamd.so
/usr/local/lib/libccolamd.so.3
/usr/local/lib/libccolamd.so.3.3.5
/usr/local/lib/libcholmod.so
/usr/local/lib/libcholmod.so.5
/usr/local/lib/libcholmod.so.5.3.4
/usr/local/lib/libcolamd.so
...
```

The complete list is `stage/<arch>/.manifest/suitesparse` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa suitesparse
ls dists/8.0/*/ | grep '^suitesparse-'
```

This writes `suitesparse-<ver>-r0.tar.gz` (run time) and `suitesparse-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add suitesparse
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/suitesparse-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/suitesparse-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | suitesparse 7.11.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/suitesparse-7.11.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | suitesparse 7.11.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/suitesparse-7.11.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **7.11.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add suitesparse=<version>`), sources in `ported/suitesparse/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates suitesparse`; port one with `scripts/port-update suitesparse`.

<details><summary>recipe.sh</summary>

```bash
VERSION=7.11.0; GIT=https://github.com/DrTimothyAldenDavis/SuiteSparse.git; REF=v7.11.0
DEPS="openblas"
CMAKE_ARGS_EXTRA=( -DSUITESPARSE_USE_FORTRAN=OFF -DSUITESPARSE_USE_CUDA=OFF -DSUITESPARSE_USE_OPENMP=OFF
                   -DBLA_VENDOR=OpenBLAS -DBLAS_LIBRARIES="$STAGE$PREFIX/lib/libopenblas.so" -DLAPACK_LIBRARIES="$STAGE$PREFIX/lib/libopenblas.so"
                   -DSUITESPARSE_DEMOS=OFF -DBUILD_TESTING=OFF -DSUITESPARSE_ENABLE_PROJECTS="suitesparse_config;amd;btf;camd;ccolamd;colamd;cholmod;cxsparse;klu;ldl;umfpack;rbio;spqr" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
