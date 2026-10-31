# openblas 0.3.28

> Optimized BLAS linear algebra library based on GotoBLAS2

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/OpenMathLib/OpenBLAS.git (tag `v0.3.28`) |
| Build system | CMake |
| QNX patches | 3 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_openblas`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add openblas
apk add openblas-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-openblas && /usr/local/bin/qnx-port-tests/test_openblas    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install openblas`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild openblas` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v0.3.28 --recurse-submodules --shallow-submodules \
    https://github.com/OpenMathLib/OpenBLAS.git src/openblas
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/openblas < ports/openblas/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/openblas < ports/openblas/patches/0002-blas-server-qnx-headers.patch
patch -p1 -d src/openblas < ports/openblas/patches/0003-qnx-cap-threads-at-cpu-count.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Synchronize libm linkage

> Imported from qnx-ports/OpenBLAS (5ef8b1964658...qnx-0.3.28); original authorship kept.
>
>
> ---
>  ctest/CMakeLists.txt | 8 ++++----
>  utest/CMakeLists.txt | 2 +-
>  2 files changed, 5 insertions(+), 5 deletions(-)

Touches: `ctest/CMakeLists.txt`, `utest/CMakeLists.txt`, `CMakeLists.txt`, `cpp_thread_test/CMakeLists.txt`, `ctest/CMakeLists.txt`, `utest/CMakeLists.txt` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-blas-server-qnx-headers.patch`](patches/0002-blas-server-qnx-headers.patch)** — Include the POSIX headers blas_server.c needs on QNX too.

> blas_server.c includes <sys/resource.h>, <signal.h>, <dlfcn.h> and friends
> only for a list of named systems; on QNX the thread-creation error path then
> fails to compile ("storage size of 'rlim' isn't known").

Touches: `driver/others/blas_server.c` · Upstream status: QNX-specific

**[`0003-qnx-cap-threads-at-cpu-count.patch`](patches/0003-qnx-cap-threads-at-cpu-count.patch)** — Limit the default thread count to the CPUs present on QNX, as on Linux.

> blas_get_cpu_number() caps the default thread count at get_num_procs() only
> for the operating systems it lists, and QNX is not among them, although
> get_num_procs() exists for it (0001, from the syspage). On QNX the default
> was therefore MAX_CPU_NUMBER -- NUM_THREADS, 64 here -- whatever the machine
> has, and every server thread takes a BUFFER_SIZE buffer, 128 MB on x86_64.
> QNX commits anonymous memory when it is mapped rather than when it is
> touched, so loading libopenblas consumed all of a 2 GB target's RAM before
> main() and the process stalled (PyTorch, which links it, never started;
> with OPENBLAS_NUM_THREADS=4 it ran in 5 s). Add OS_QNX to both lists.

Touches: `driver/others/memory.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/openblas -B build/$QNX_ARCH/openblas -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DNOFORTRAN=ON \
      -DC_LAPACK=ON \
      -DBUILD_WITHOUT_LAPACK=OFF \
      -DDYNAMIC_ARCH=OFF \
      -DUSE_OPENMP=0 \
      -DUSE_THREAD=1 \
      -DNUM_THREADS=64 \
      -DBUILD_TESTING=OFF \
      -DTARGET=$([ \
      $QNX_ARCH" = aarch64le ] && echo ARMV8 || echo HASWELL)"
cmake --build build/$QNX_ARCH/openblas -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/openblas
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Built with OpenBLAS's CMake build, as qnx-ports does: the Makefile build queries the compiler with -dumpversion, which qcc does not answer (and then fails with "expr: syntax error"). No Fortran: LAPACK comes from its C translation (C_LAPACK). One kernel set per architecture -- ARMV8 (NEON) and HASWELL (AVX2) -- rather than runtime dispatch. The default thread count is capped at the CPU count by patch 0003 -- on QNX it was 64, each with a 128 MB buffer QNX commits at once. The buffer size stays (a smaller one makes OpenBLAS warn that large GEMMs may crash).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH openblas`.

### 5. What it installs

16 files under `/usr/local`: 3 shared-library entries, 8 headers.

```
/usr/local/lib/libopenblas.so
/usr/local/lib/libopenblas.so.0
/usr/local/lib/libopenblas.so.0.3
```

The complete list is `stage/<arch>/.manifest/openblas` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa openblas
ls dists/8.0/*/ | grep '^openblas-'
```

This writes `openblas-<ver>-r0.tar.gz` (run time) and `openblas-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add openblas
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/openblas-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/openblas-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_openblas`](../../tests/src/) links openblas the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-openblas`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-openblas
/usr/local/bin/qnx-port-tests/test_openblas
```

```
== openblas ==
  ok   ...
PASS openblas
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_openblas` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_openblas`](../../tests/src/) -- a program linked against openblas the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - thread count reported
   - ddot([1..4],[5..8]) = 70
   - daxpy y += 2x
   - dnrm2([3,4]) = 5
   - idamax finds |-9|
   - dgemv [[1,2,3],[4,5,6]]*[1,0,-1] = [-2,-2]
   - dgemm 2x3 * 3x2 = [58 64; 139 154]
   - sgemm column-major 2x2 product
   - dgemm 160x160 matches the naive product exactly
   - LAPACKE_dgesv solves a 3x3 system
   - LAPACKE_dsyev: tridiag(-1,2,-1) eigenvalues 2-sqrt2, 2, 2+sqrt2
   - dgetrf+dgetri inverse of [[4,7],[2,6]]
   - dgesv reports a singular matrix

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | openblas 0.3.28-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/openblas-0.3.28-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | openblas 0.3.28-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/openblas-0.3.28-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | openblas 0.3.28-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/openblas-0.3.28-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | openblas 0.3.28-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/openblas-0.3.28-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.3.28**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add openblas=<version>`), sources in `ported/openblas/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates openblas`; port one with `scripts/port-update openblas`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.3.28; GIT=https://github.com/OpenMathLib/OpenBLAS.git; REF=v0.3.28
# Built with OpenBLAS's CMake build, as qnx-ports does: the Makefile build
# queries the compiler with -dumpversion, which qcc does not answer (and then
# fails with "expr: syntax error"). No Fortran: LAPACK comes from its C
# translation (C_LAPACK). One kernel set per architecture -- ARMV8 (NEON) and
# HASWELL (AVX2) -- rather than runtime dispatch.
# The default thread count is capped at the CPU count by patch 0003 -- on
# QNX it was 64, each with a 128 MB buffer QNX commits at once. The buffer
# size stays (a smaller one makes OpenBLAS warn that large GEMMs may crash).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DNOFORTRAN=ON -DC_LAPACK=ON -DBUILD_WITHOUT_LAPACK=OFF
                   -DDYNAMIC_ARCH=OFF -DUSE_OPENMP=0 -DUSE_THREAD=1 -DNUM_THREADS=64 -DBUILD_TESTING=OFF
                   "-DTARGET=$([ "$QNX_ARCH" = aarch64le ] && echo ARMV8 || echo HASWELL)" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
