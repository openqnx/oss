# openblas 0.3.28

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/OpenMathLib/OpenBLAS.git (tag `v0.3.28`) |
| Build system | CMake |
| QNX patches | 3 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install openblas
oss-get install openblas-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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
./scripts/qnxpkg -a $QNX_ARCH openblas
ls repo/8.0/*/ | grep '^openblas-'
```

This writes `openblas-<ver>-r0.tar.gz` (run time) and `openblas-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install openblas
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/openblas-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/openblas-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

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
