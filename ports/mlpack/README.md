# mlpack 4.8.0

> C++ machine learning library and command-line programs (on Armadillo)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/mlpack/mlpack.git (tag `4.8.0`) |
| Build system | CMake |
| QNX patches | 3 |
| Build dependencies | [`armadillo`](../armadillo/) [`ensmallen`](../ensmallen/) [`cereal`](../cereal/) |
| Run-time dependencies | [`armadillo`](../armadillo/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_mlpack_cli`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `mlpack` (programs, shared libraries, run-time data) and `mlpack-dev` (headers, pkg-config/CMake files, static libraries; needs `mlpack`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`mlpack`](https://openqnx.github.io/oss/package/x86_64/mlpack.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/mlpack.html)) · [`mlpack-dev`](https://openqnx.github.io/oss/package/x86_64/mlpack-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/mlpack-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add mlpack` (dependencies come along); one version: `apk add mlpack=<version>` | `apk del mlpack` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install mlpack` (dependencies first) | `oss-get remove mlpack` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`mlpack-dev` the same way, by its own name (`apk add mlpack-dev`, `oss-get install mlpack-dev`, by hand with its own file and list `mlpack-dev.list`); remove it before `mlpack`, which it needs.

```sh
# on the QNX target, as root
apk add mlpack mlpack-dev
apk add qnx-test-mlpack-cli && /usr/local/bin/qnx-port-tests/test_mlpack_cli    # its on-target test, from tests-8.0-<arch>-m-z
apk del mlpack mlpack-dev qnx-test-mlpack-cli
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/mlpack.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/mlpack.list
rm var/lib/manual/mlpack.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild mlpack` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (4 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild armadillo ensmallen cereal
```

<details><summary>Full build chain (4)</summary>

[`armadillo`](../armadillo/) [`cereal`](../cereal/) [`ensmallen`](../ensmallen/) [`openblas`](../openblas/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 4.8.0 --recurse-submodules --shallow-submodules \
    https://github.com/mlpack/mlpack.git src/mlpack
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/mlpack < ports/mlpack/patches/0001-cross-build-not-embedded.patch
patch -p1 -d src/mlpack < ports/mlpack/patches/0002-find-own-headers-cross.patch
patch -p1 -d src/mlpack < ports/mlpack/patches/0003-pkgconfig-no-source-dir.patch
```

**[`0001-cross-build-not-embedded.patch`](patches/0001-cross-build-not-embedded.patch)** — Make mlpack's embedded cross-compile mode an option.

> mlpack takes every cross build for one for a small embedded board: it adds
> size flags (-Os -s -flto -fvisibility=hidden -fno-unwind-tables ...),
> requires TOOLCHAIN_PREFIX, ignores the installed Armadillo/BLAS and
> downloads and statically builds OpenBLAS and Armadillo itself, and defaults
> to static libraries. A QNX target is a full OS with OpenBLAS, Armadillo,
> ensmallen and cereal installed. The new option CROSSCOMPILE_EMBEDDED
> (default: CMAKE_CROSSCOMPILING, so nothing changes for anyone else) selects
> that mode; the QNX build sets it OFF and is configured like a native one.

Touches: `CMake/ConfigureCrossCompile.cmake`, `CMakeLists.txt` · Upstream status: Pending

**[`0002-find-own-headers-cross.patch`](patches/0002-find-own-headers-cross.patch)** — Find mlpack's own source headers in a cross build.

> In a cross build the toolchain file sets CMAKE_FIND_ROOT_PATH_MODE_INCLUDE
> to ONLY, so find_path() re-roots every search path under the target
> sysroot -- including mlpack's own source tree, where the bindings look for
> mlpack.hpp -- and configuring failed with "mlpack not found!". The source
> tree is a build-machine path: search it as it is.

Touches: `CMake/mlpack.cmake` · Upstream status: Pending

**[`0003-pkgconfig-no-source-dir.patch`](patches/0003-pkgconfig-no-source-dir.patch)** — Keep the source tree out of mlpack.pc's Cflags.

> mlpack.pc's Cflags are built from MLPACK_INCLUDE_DIRS, which during the
> build also holds mlpack's own source tree (src/, where the bindings find
> mlpack.hpp). The installed mlpack.pc then pointed consumers at the build
> machine's source directory (-I/home/.../src/mlpack/src) in front of the
> installed headers. Leave the source tree out; the install prefix's include
> directory is added right after.

Touches: `CMakeLists.txt` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/mlpack -B build/$QNX_ARCH/mlpack -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCROSSCOMPILE_EMBEDDED=OFF \
      -DUSE_OPENMP=OFF \
      -DBUILD_CLI_EXECUTABLES=ON \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TESTS=OFF \
      -DBUILD_PYTHON_BINDINGS=OFF \
      -DBUILD_JULIA_BINDINGS=OFF \
      -DBUILD_R_BINDINGS=OFF \
      -DBUILD_GO_BINDINGS=OFF \
      -DDOWNLOAD_DEPENDENCIES=OFF
cmake --build build/$QNX_ARCH/mlpack -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/mlpack
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- mlpack: C++ machine learning library (k-means, decision trees, random forests, kNN, logistic/linear regression, PCA, neural networks, ...) on Armadillo, with its command-line programs (mlpack_kmeans, mlpack_knn, mlpack_linear_regression, ...). The library itself is header-only.
- Armadillo (on OpenBLAS) for the matrices, ensmallen for the optimisers, cereal for model serialisation (the CLI programs' --input_model/--output_model).
- CROSSCOMPILE_EMBEDDED=OFF (patch 0001): configure like a native build against the installed dependencies, not mlpack's embedded-board mode. OpenMP off: the SDP has no libgomp. The CLI programs are built (shared, linking libarmadillo); Python/Julia/R/Go bindings and the test suite are not. STB, dr_libs (image/audio loading) stay the copies bundled in the headers.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH mlpack`.

### 5. What it installs

1197 files under `/usr/local`, 48 programs, 1148 headers.

```
/usr/local/bin/mlpack_adaboost
/usr/local/bin/mlpack_approx_kfn
/usr/local/bin/mlpack_bayesian_linear_regression
/usr/local/bin/mlpack_cf
/usr/local/bin/mlpack_dbscan
/usr/local/bin/mlpack_decision_tree
/usr/local/bin/mlpack_det
/usr/local/bin/mlpack_emst
/usr/local/bin/mlpack_fastmks
/usr/local/bin/mlpack_gmm_generate
/usr/local/bin/mlpack_gmm_probability
/usr/local/bin/mlpack_gmm_train
/usr/local/bin/mlpack_hmm_generate
/usr/local/bin/mlpack_hmm_loglik
/usr/local/bin/mlpack_hmm_train
/usr/local/bin/mlpack_hmm_viterbi
...
```

The complete list is `stage/<arch>/.manifest/mlpack` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa mlpack
ls dists/8.0/*/ | grep '^mlpack-'
```

This writes `mlpack-<ver>-r0.tar.gz` (run time) and `mlpack-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add mlpack
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/mlpack-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/mlpack-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_mlpack_cli`](../../tests/src/) links mlpack the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-mlpack-cli`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-mlpack-cli
/usr/local/bin/qnx-port-tests/test_mlpack_cli
```

```
== mlpack ==
  ok   ...
PASS mlpack
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_mlpack_cli` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_mlpack_cli`](../../tests/src/) -- a program linked against mlpack the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> mlpack`.

## Versions

This recipe builds **4.8.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add mlpack=<version>`), sources in `ported/mlpack/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates mlpack`; port one with `scripts/port-update mlpack`.

<details><summary>recipe.sh</summary>

```bash
# mlpack: C++ machine learning library (k-means, decision trees, random
# forests, kNN, logistic/linear regression, PCA, neural networks, ...) on
# Armadillo, with its command-line programs (mlpack_kmeans, mlpack_knn,
# mlpack_linear_regression, ...). The library itself is header-only.
VERSION=4.8.0; GIT=https://github.com/mlpack/mlpack.git; REF=4.8.0
# Armadillo (on OpenBLAS) for the matrices, ensmallen for the optimisers,
# cereal for model serialisation (the CLI programs' --input_model/--output_model).
DEPS="armadillo ensmallen cereal"
BUILDSYS=cmake
# CROSSCOMPILE_EMBEDDED=OFF (patch 0001): configure like a native build
# against the installed dependencies, not mlpack's embedded-board mode.
# OpenMP off: the SDP has no libgomp. The CLI programs are built (shared,
# linking libarmadillo); Python/Julia/R/Go bindings and the test suite are not.
# STB, dr_libs (image/audio loading) stay the copies bundled in the headers.
CMAKE_ARGS_EXTRA=( -DCROSSCOMPILE_EMBEDDED=OFF -DUSE_OPENMP=OFF -DBUILD_CLI_EXECUTABLES=ON -DBUILD_SHARED_LIBS=ON
                   -DBUILD_TESTS=OFF -DBUILD_PYTHON_BINDINGS=OFF -DBUILD_JULIA_BINDINGS=OFF -DBUILD_R_BINDINGS=OFF
                   -DBUILD_GO_BINDINGS=OFF -DDOWNLOAD_DEPENDENCIES=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
