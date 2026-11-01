# openmp 22.1.8

> LLVM OpenMP runtime library (libomp)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/llvm/llvm-project.git (tag `llvmorg-22.1.8`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_openmp`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `openmp` (programs, shared libraries, run-time data) and `openmp-dev` (headers, pkg-config/CMake files, static libraries; needs `openmp`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`openmp`](https://openqnx.github.io/oss/package/x86_64/openmp.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/openmp.html)) · [`openmp-dev`](https://openqnx.github.io/oss/package/x86_64/openmp-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/openmp-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add openmp` (dependencies come along); one version: `apk add openmp=<version>` | `apk del openmp` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install openmp` (dependencies first) | `oss-get remove openmp` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`openmp-dev` the same way, by its own name (`apk add openmp-dev`, `oss-get install openmp-dev`, by hand with its own file and list `openmp-dev.list`); remove it before `openmp`, which it needs.

```sh
# on the QNX target, as root
apk add openmp openmp-dev
apk add qnx-test-openmp && /usr/local/bin/qnx-port-tests/test_openmp    # its on-target test, from tests-8.0-<arch>-m-z
apk del openmp openmp-dev qnx-test-openmp
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/openmp.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/openmp.list
rm var/lib/manual/openmp.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild openmp` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b llvmorg-22.1.8 --recurse-submodules --shallow-submodules \
    https://github.com/llvm/llvm-project.git src/openmp
```

The project lives in the `openmp/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/openmp/openmp -B build/$QNX_ARCH/openmp -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DOPENMP_STANDALONE_BUILD=ON \
      -DOPENMP_ENABLE_LIBOMPTARGET=OFF \
      -DLIBOMP_OMPD_SUPPORT=OFF \
      -DLIBOMP_FORTRAN_MODULES=OFF \
      -DLIBOMP_INSTALL_ALIASES=OFF \
      -DOPENMP_ENABLE_TESTING=OFF \
      -DLIBOMP_ENABLE_SHARED=ON \
      -DLIBOMP_USE_HWLOC=OFF \
      -DOPENMP_ENABLE_OMPT_TOOLS=OFF \
      -DLIBOMP_LIBFLAGS=-lcatalog \
      -DPython3_EXECUTABLE=/usr/bin/python3
cmake --build build/$QNX_ARCH/openmp -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/openmp
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- LLVM's OpenMP runtime, libomp, from the same llvm-project tree and QNX patches as the llvm component (ports/llvm: qnx-ports' fork, whose "qnx/openmp: add qnx target support" commit is the QNX port of libomp), as a component of its own so that llvm is not rebuilt. Built standalone from openmp/ with the SDP's GCC. libomp also implements GCC's GOMP_* entry points, and the SDP's GCC has no libgomp: code compiled with "qcc -fopenmp" links against libomp (-fopenmp to compile, -lomp to link), as does clang -fopenmp on the target.
- The llvm component's source tree is shared (it has these patches applied). Fetched here only when it is not there yet: llvm-project at the same tag with ports/llvm/patches -- the tree the llvm component builds from.
- No offloading (libomptarget: GPUs), no OMPD debugging library (needs gdb-plugin pieces and Python), no Fortran module. The GNU/Intel name aliases (libgomp.so, libiomp5.so symlinks) are not installed: a libgomp.so that is libomp would shadow GCC's should it ever be added. Python is the build machine's: libomp generates its export lists with a script; the staging tree's python3 is a QNX program. -lcatalog: libomp reads its messages with catopen()/catgets(), which QNX has in libcatalog, not libc (unlinked, every program using libomp failed to link on them).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH openmp`.

### 5. What it installs

5 files under `/usr/local`: 1 shared-library entries, 4 headers.

```
/usr/local/lib/libomp.so
```

The complete list is `stage/<arch>/.manifest/openmp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa openmp
ls dists/8.0/*/ | grep '^openmp-'
```

This writes `openmp-<ver>-r0.tar.gz` (run time) and `openmp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add openmp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/openmp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/openmp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_openmp`](../../tests/src/) links openmp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-openmp`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-openmp
/usr/local/bin/qnx-port-tests/test_openmp
```

```
== openmp ==
  ok   ...
PASS openmp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_openmp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_openmp`](../../tests/src/) -- a program linked against openmp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - the runtime sees >= 1 processor
   - serial part: not in parallel, thread 0
   - parallel for reduction(+) over 1..1000000 = 500000500000
   - a team of num_threads(4) has 4 threads
   - thread numbers 0..3, each once (critical section)
   - omp_in_parallel() inside the region
   - schedule(dynamic,7): 1000 iterations, each exactly once (atomic)
   - tasks: fib(20) = 6765
   - omp_get_wtime / omp_get_wtick
   - omp_set_num_threads(3) gives a team of 3

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | openmp 22.1.8-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/openmp-22.1.8-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | openmp 22.1.8-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/openmp-22.1.8-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **22.1.8**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add openmp=<version>`), sources in `ported/openmp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates openmp`; port one with `scripts/port-update openmp`.

<details><summary>recipe.sh</summary>

```bash
VERSION=22.1.8; GIT=https://github.com/llvm/llvm-project.git; REF=llvmorg-22.1.8
# LLVM's OpenMP runtime, libomp, from the same llvm-project tree and QNX
# patches as the llvm component (ports/llvm: qnx-ports' fork, whose
# "qnx/openmp: add qnx target support" commit is the QNX port of libomp), as
# a component of its own so that llvm is not rebuilt. Built standalone from
# openmp/ with the SDP's GCC.
# libomp also implements GCC's GOMP_* entry points, and the SDP's GCC has no
# libgomp: code compiled with "qcc -fopenmp" links against libomp
# (-fopenmp to compile, -lomp to link), as does clang -fopenmp on the target.
SRCROOT="$SRC_DIR/llvm"
# The llvm component's source tree is shared (it has these patches applied).
# Fetched here only when it is not there yet: llvm-project at the same tag
# with ports/llvm/patches -- the tree the llvm component builds from.
do_fetch() {
  git clone -q --depth 1 --branch "$REF" "$GIT" "$SRCROOT" || return 1
  local p; for p in "$WS"/ports/llvm/patches/*.patch; do
    echo "-- patch (llvm) $(basename "$p")"; patch -p1 -d "$SRCROOT" < "$p" || { rm -rf "$SRCROOT"; return 1; }
  done
}
SUBDIR=openmp
# No offloading (libomptarget: GPUs), no OMPD debugging library (needs
# gdb-plugin pieces and Python), no Fortran module. The GNU/Intel name
# aliases (libgomp.so, libiomp5.so symlinks) are not installed: a libgomp.so
# that is libomp would shadow GCC's should it ever be added. Python is the
# build machine's: libomp generates its export lists with a script; the
# staging tree's python3 is a QNX program. -lcatalog: libomp reads its
# messages with catopen()/catgets(), which QNX has in libcatalog, not libc
# (unlinked, every program using libomp failed to link on them).
CMAKE_ARGS_EXTRA=( -DOPENMP_STANDALONE_BUILD=ON -DOPENMP_ENABLE_LIBOMPTARGET=OFF -DLIBOMP_OMPD_SUPPORT=OFF
                   -DLIBOMP_FORTRAN_MODULES=OFF -DLIBOMP_INSTALL_ALIASES=OFF -DOPENMP_ENABLE_TESTING=OFF
                   -DLIBOMP_ENABLE_SHARED=ON -DLIBOMP_USE_HWLOC=OFF -DOPENMP_ENABLE_OMPT_TOOLS=OFF
                   -DLIBOMP_LIBFLAGS=-lcatalog -DPython3_EXECUTABLE=/usr/bin/python3 )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
