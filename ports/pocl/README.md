# pocl 7.2

> Portable Computing Language: OpenCL 3.0 on the CPU, kernels compiled at run time with Clang/LLVM

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/pocl/pocl.git (tag `v7.2`) |
| Build system | custom (see recipe) |
| QNX patches | 5 |
| Build dependencies | [`llvm`](../llvm/) [`zlib`](../zlib/) [`zstd`](../zstd/) [`opencl-headers`](../opencl-headers/) [`opencl-icd-loader`](../opencl-icd-loader/) |
| Run-time dependencies | [`llvm`](../llvm/) [`opencl-icd-loader`](../opencl-icd-loader/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_pocl`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `pocl` (programs, shared libraries, run-time data) and `pocl-dev` (headers, pkg-config/CMake files, static libraries; needs `pocl`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`pocl`](https://openqnx.github.io/oss/package/x86_64/pocl.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/pocl.html)) · [`pocl-dev`](https://openqnx.github.io/oss/package/x86_64/pocl-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/pocl-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add pocl` (dependencies come along); one version: `apk add pocl=<version>` | `apk del pocl` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install pocl` (dependencies first) | `oss-get remove pocl` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`pocl-dev` the same way, by its own name (`apk add pocl-dev`, `oss-get install pocl-dev`, by hand with its own file and list `pocl-dev.list`); remove it before `pocl`, which it needs.

```sh
# on the QNX target, as root
apk add pocl pocl-dev
apk add qnx-test-pocl && /usr/local/bin/qnx-port-tests/test_pocl    # its on-target test, from tests-8.0-<arch>-m-z
apk del pocl pocl-dev qnx-test-pocl
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/pocl.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/pocl.list
rm var/lib/manual/pocl.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pocl` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (19 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild llvm zlib zstd opencl-headers opencl-icd-loader
```

<details><summary>Full build chain (19)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`llvm`](../llvm/) [`ncurses`](../ncurses/) [`opencl-headers`](../opencl-headers/) [`opencl-icd-loader`](../opencl-icd-loader/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v7.2 --recurse-submodules --shallow-submodules \
    https://github.com/pocl/pocl.git src/pocl
```

### 3. Apply the QNX patches

5 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/pocl < ports/pocl/patches/0001-qnx-syspage-device-info.patch
patch -p1 -d src/pocl < ports/pocl/patches/0002-qnx-compiler-stack.patch
patch -p1 -d src/pocl < ports/pocl/patches/0003-qnx-netbyte-h.patch
patch -p1 -d src/pocl < ports/pocl/patches/0004-cross-target-llvm-bindir.patch
patch -p1 -d src/pocl < ports/pocl/patches/0005-qnx-datadir-fallback.patch
```

**[`0001-qnx-syspage-device-info.patch`](patches/0001-qnx-syspage-device-info.patch)** — Describe the CPU device from the QNX system page (CPUs, RAM, cache, CPU name and speed).

> The CPU device takes its description from Linux files: the number of CPUs
> from /sys/devices/system/cpu/possible, the memory size from /proc/meminfo,
> the cache size from /sys/devices/system/cpu/cpu0/cache, the CPU's name and
> clock frequency from /proc/cpuinfo. Without hwloc and off Linux it falls
> back to constants: 4 compute units and 2 GiB of memory whatever the machine
> has -- a pthread driver with 4 threads on a single-core board, and
> CL_DEVICE_GLOBAL_MEM_SIZE / CL_DEVICE_MAX_MEM_ALLOC_SIZE that have nothing
> to do with the RAM installed -- and no CPU name or frequency at all.
>
> QNX has none of those files; the same facts are in the system page
> (<sys/syspage.h>), which every process has mapped: num_cpu, the "ram"
> entries of the address space table (asinfo), each CPU's chain of data caches
> (cacheattr), and per CPU its speed in MHz and the name the startup program
> gave it (cpuinfo). The device now reports those: CL_DEVICE_MAX_COMPUTE_UNITS
> is the number of CPUs, the memory sizes follow the RAM, the device name is
> "cpu-<llvm cpu>-<CPU name>" as on Linux, CL_DEVICE_MAX_CLOCK_FREQUENCY the
> CPU's speed. The platform version string ends in "QNX" instead of "Unix".

Touches: `lib/CL/clGetPlatformInfo.c`, `lib/CL/devices/cpuinfo.c`, `lib/CL/devices/topology/pocl_topology.c` · Upstream status: QNX-specific

**[`0002-qnx-compiler-stack.patch`](patches/0002-qnx-compiler-stack.patch)** — Run the kernel compiler on an 8 MiB stack: QNX thread stacks are 256-512 KiB.

> On QNX the main thread's stack is 512 KiB and every other thread gets
> 256 KiB unless it asks for more; on Linux both have 8 MiB. PoCL runs Clang
> (parser, semantic analysis, code generation), LLVM's optimisation passes
> and instruction selection, and lld inside the thread that calls
> clBuildProgram()/clCompileProgram()/clLinkProgram(), and again -- for the
> work-group function of each kernel -- inside its driver threads or, with
> the cpu-minimal driver, inside the thread that calls clFinish(). All of
> that code is recursive over the program it compiles and assumes the large
> stack (the clang program itself runs each compilation on a thread with
> 8 MiB for this reason). On the QNX default stacks a build overflows the
> stack and the process dies with SIGSEGV, at a depth that depends on the
> kernel source.
>
> On QNX:
>  - the threads PoCL creates (POCL_CREATE_THREAD: the pthread driver's
>    workers) get an 8 MiB stack;
>  - compile_and_link_program() and the kernel code generation
>    (llvm_codegen()) are entered through pocl_run_with_large_stack(), which
>    calls the function directly when the calling thread already has 8 MiB
>    (its size is in the thread's local storage block, __tls()->__stacksize)
>    and otherwise runs it on a thread created for the call and waits for it.
>    The program build callback (pfn_notify) is then called from that thread,
>    which OpenCL allows;
>  - pocl_get_thread_stack_size() returns the real size instead of 0.
> Thread stacks are mapped lazily on QNX, so the larger limit costs memory
> only where it is used. Other systems are not changed.

Touches: `lib/CL/devices/common.c`, `lib/CL/pocl_build.c`, `lib/CL/pocl_threads.c`, `lib/CL/pocl_threads_c.h` · Upstream status: QNX-specific

**[`0003-qnx-netbyte-h.patch`](patches/0003-qnx-netbyte-h.patch)** — Take htole32() and friends from <net/netbyte.h> on QNX, which has no <endian.h>.

> pocl_cl.h includes <endian.h> for htole16() ... le64toh() on every system
> that is not macOS, FreeBSD or Windows. QNX has no <endian.h> ("fatal error:
> endian.h: No such file or directory" in every C++ file of lib/llvmopencl);
> it declares the same functions in <net/netbyte.h>.

Touches: `lib/CL/pocl_cl.h` · Upstream status: QNX-specific

**[`0004-cross-target-llvm-bindir.patch`](patches/0004-cross-target-llvm-bindir.patch)** — Let a cross build name the target's LLVM program directory (POCL_TARGET_LLVM_BINDIR).

> In a cross build PoCL looks for the target's clang, opt, llc ... in the
> directory LLVMConfig.cmake reports (LLVM_TOOLS_BINARY_DIR), requires them
> to exist there, and then strips CMAKE_SYSROOT from what it found to get
> the path on the target, which it compiles in (CLANGCC, the name the Clang
> driver is given). That assumes the target's LLVM is installed inside
> CMAKE_SYSROOT. Here CMAKE_SYSROOT is the QNX SDP and the target's LLVM is
> in a staging tree outside it; the toolchain file also keeps the staging
> tree's bin directory out of every program search, because the programs in
> it are QNX binaries that cannot run on the build machine. The configure
> step stopped with "clang executable not found in <stage>/usr/local/bin!",
> and had it found them the staging path would have been compiled into
> libpocl.
>
> POCL_TARGET_LLVM_BINDIR=<dir> names the directory the LLVM programs have on
> the target; the names are recorded without searching. Without it nothing
> changes.

Touches: `cmake/LLVM.cmake` · Upstream status: QNX-specific

**[`0005-qnx-datadir-fallback.patch`](patches/0005-qnx-datadir-fallback.patch)** — Fall back to the configured data directory when dladdr() gives no directory.

> libpocl finds its data (the kernel library bitcode in share/pocl) relative
> to its own file, which it asks dladdr() for. When the loader knows the
> library by a bare name only -- no directory in dli_fname -- the lookup
> gave up and returned an error, although the configured directory
> (POCL_INSTALL_PRIVATE_DATADIR) would have been right. On QNX fall back to
> the configured directory in that case, as when relocation is disabled.

Touches: `lib/CL/pocl_util.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/pocl; B=build/$QNX_ARCH/pocl; mkdir -p $B
```

```bash
do_configure() {
  pocl_host_llvm || return 1
  local cpu=x86-64; [ "$QNX_CPU" = aarch64 ] && cpu=generic
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DWITH_LLVM_CONFIG="$BUILD_DIR/host/pocl-llvm-$POCL_LLVM/bin/llvm-config" -DLLVM_DIR="$STAGE$PREFIX/lib/cmake/llvm" \
    -DPOCL_TARGET_LLVM_BINDIR="$PREFIX/bin" -DSTATIC_LLVM=OFF -DLLC_HOST_CPU="$cpu" -DHOST_CPU_CACHELINE_SIZE=64 \
    -DENABLE_HOST_CPU_DEVICES=ON -DENABLE_LOADABLE_DRIVERS=OFF -DENABLE_HWLOC=OFF -DENABLE_SPIRV=OFF -DENABLE_HOST_CPU_VECTORIZE_SLEEF=OFF -DCMAKE_DISABLE_FIND_PACKAGE_onnxruntime=ON -DCMAKE_DISABLE_FIND_PACKAGE_LIBJPEG_TURBO=ON -DCMAKE_DISABLE_FIND_PACKAGE_OpenCV=ON \
    -DENABLE_ICD=ON -DPOCL_ICD_ABSOLUTE_PATH=OFF -DPOCL_INSTALL_ICD_VENDORDIR="$PREFIX/etc/OpenCL/vendors" \
    -DINSTALL_OPENCL_HEADERS=OFF -DENABLE_TESTS=OFF -DENABLE_EXAMPLES=OFF -DENABLE_POCLCC=ON -DENABLE_POCL_BUILDING=OFF
}
```

```bash
do_build() { cmake --build "$B" -j4; }
```

```bash
do_install() {
  DESTDIR="$STAGE" cmake --install "$B"
  install -Dm644 "$S/LICENSE" "$STAGE$PREFIX/share/licenses/pocl/LICENSE"
  install -m644 "$S/LICENSE.with.3rdparty" "$STAGE$PREFIX/share/licenses/pocl/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- pocl (Portable Computing Language): an OpenCL 3.0 implementation that runs on the CPU. OpenCL C kernels are compiled on the target when a program is built (clBuildProgram): Clang and LLVM -- the llvm component's libclang-cpp.so and libLLVM.so, inside libpocl -- compile the source, PoCL's passes turn each kernel into a loop over its work group, LLVM generates code for the CPU and lld (linked into libpocl) makes a shared object, which the driver loads and runs on one thread per CPU ("cpu" device, pthread driver; POCL_DEVICES=cpu-minimal selects the single threaded one). Compiled kernels are cached in $POCL_CACHE_DIR (default $HOME/.cache/pocl/kcache, /tmp/pocl/kcache without HOME). Installed as an ICD for opencl-icd-loader: libpocl.so.2 and etc/OpenCL/vendors/pocl.icd, which names the library without a directory, so the loader's dlopen() finds it wherever the package root is. Programs link libOpenCL.so.1 and find the platform "Portable Computing Language". Also poclcc (compiles a kernel file to a PoCL binary ahead of time) and the built-in function library the kernels are linked with (share/pocl/kernel-<triple>-<cpu>.bc, LLVM bitcode). 7.2 is the release for LLVM 18 to 23; the llvm component is 22.1.8. How it is built for QNX: - cross build as upstream supports it: LLVM_DIR is the target's LLVM (staging), WITH_LLVM_CONFIG a host LLVM of the same release whose clang compiles the built-in function library for the target triple (pocl_host_llvm below builds those host tools once, from the llvm component's source). The target's LLVM programs are named, not searched for (patches/0004): the staging tree is not inside CMAKE_SYSROOT. - the target CPU is fixed: x86-64 (the SSE2 baseline) and generic (ARMv8-A with NEON) -- LLC_HOST_CPU. PoCL otherwise asks llc on the build machine for its CPU, and at run time LLVM for the host CPU, which it cannot identify on QNX/aarch64 (no /proc/cpuinfo); kernels compiled for the baseline run on every target and in the QEMU test machines. POCL_LLVM_CPU_NAME is not consulted with a fixed CPU. - the device's description comes from the QNX system page: number of CPUs, RAM, cache size, CPU name and speed (patches/0001); hwloc is not used (not ported). - the compiler runs on an 8 MiB stack (patches/0002): QNX threads have 256-512 KiB by default, Clang and LLVM need more. - <net/netbyte.h> for the byte order functions (patches/0003); the data directory lookup tolerates a library name without a directory (patches/0005). - the drivers are linked into libpocl (ENABLE_LOADABLE_DRIVERS=OFF: one library, nothing to find at run time); LLVM and Clang are the shared libraries of the llvm component, lld's ELF linker is linked statically (the component has no shared lld); zlib and zstd are lld's. - ENABLE_POCL_BUILDING=OFF: the "run from the build tree" mode would compile the build machine's source and build paths into libpocl. - left out: SPIR-V input (needs the SPIRV-LLVM-Translator, not ported), the optional built-in kernels on onnxruntime, OpenCV and libjpeg-turbo (found in staging otherwise: undeclared dependencies), vectorised math through the sleef component's library on aarch64 (PoCL would compile the library's staging path into the kernel link line), the remote, Vulkan, CUDA and Level Zero drivers, tests and examples (they run on the build machine). PoCL's copies of the OpenCL headers are not installed: opencl-headers has them.
- Host LLVM tools of the same release as the target's libLLVM/libclang-cpp: pocl's kernel library (the OpenCL C built-in functions, as LLVM bitcode) is compiled at build time, for the target triple, by a clang that runs on the build machine, and the target's LLVM reads it at run time. Built once, from the llvm component's (QNX-patched) source so that it knows the same QNX triples, with the X86 and AArch64 back ends only and without optimisation (-O0 halves the build time of some 3000 files; these tools only compile the few hundred small files of the kernel library). Without optimisation GCC keeps a debug-only helper (printSpillReloadChain in MachineCopyPropagation.cpp) that nothing calls in a release build but that refers to MachineInstr::dump(), which a release build does not define: the symbol is given the address 0 at link time (--defsym).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pocl`.

### 5. What it installs

22 files under `/usr/local`: 3 shared-library entries, 1 programs.

```
/usr/local/lib/libpocl.so
/usr/local/lib/libpocl.so.2
/usr/local/lib/libpocl.so.2.16.0
/usr/local/bin/poclcc
```

The complete list is `stage/<arch>/.manifest/pocl` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa pocl
ls dists/8.0/*/ | grep '^pocl-'
```

This writes `pocl-<ver>-r0.tar.gz` (run time) and `pocl-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add pocl
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/pocl-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/pocl-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_pocl`](../../tests/src/) links pocl the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-pocl`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-pocl
/usr/local/bin/qnx-port-tests/test_pocl
```

```
== pocl ==
  ok   ...
PASS pocl
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_pocl` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_pocl`](../../tests/src/) -- a program linked against pocl the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - scratch directory
   - clGetPlatformIDs finds a platform
   - the platform \"Portable Computing Language\" is there
   - CL_PLATFORM_VERSION is \"OpenCL <version> PoCL ...\"
   - it has a CPU device
   - CL_DEVICE_TYPE has CL_DEVICE_TYPE_CPU
   - CL_DEVICE_MAX_COMPUTE_UNITS is at least 1
   - the device and its compiler are available
   - CL_DEVICE_OPENCL_C_VERSION is \"OpenCL C <version> ...\"
   - clCreateContext
   - clCreateCommandQueue
   - a source with an undeclared identifier: CL_BUILD_PROGRAM_FAILURE
   - the build log has the compiler's error naming the identifier
   - clCreateProgramWithSource
   - clBuildProgram compiles five kernels
   - CL_PROGRAM_NUM_KERNELS is 5
   - an unknown kernel name: CL_INVALID_KERNEL_NAME
   - clCreateKernel(saxpy)
   - saxpy is enqueued and finishes
   - saxpy: y[i] = 3*i + (4096 - i) for all 4096 elements
   - saxpy: y[1000] = 6096
   - copy + fill + map: 100 elements 2.5, the rest the copied saxpy result
   - squares: out[i] = i*i + 7 for 10000 work items
   - squares: the sum is 333283405000
   - the builtins kernel runs
   - ... and 10 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | pocl 7.2-r0 | ❌ FAIL | 2026-11-13 | [log](../../tests/logs/aarch64le/pocl-7.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | pocl 7.2-r0 | ❌ FAIL | 2026-11-13 | [log](../../tests/logs/x86_64/pocl-7.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **7.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add pocl=<version>`), sources in `ported/pocl/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates pocl`; port one with `scripts/port-update pocl`.

<details><summary>recipe.sh</summary>

```bash
# pocl (Portable Computing Language): an OpenCL 3.0 implementation that runs
# on the CPU. OpenCL C kernels are compiled on the target when a program is
# built (clBuildProgram): Clang and LLVM -- the llvm component's
# libclang-cpp.so and libLLVM.so, inside libpocl -- compile the source,
# PoCL's passes turn each kernel into a loop over its work group, LLVM
# generates code for the CPU and lld (linked into libpocl) makes a shared
# object, which the driver loads and runs on one thread per CPU ("cpu"
# device, pthread driver; POCL_DEVICES=cpu-minimal selects the single
# threaded one). Compiled kernels are cached in $POCL_CACHE_DIR (default
# $HOME/.cache/pocl/kcache, /tmp/pocl/kcache without HOME).
#
# Installed as an ICD for opencl-icd-loader: libpocl.so.2 and
# etc/OpenCL/vendors/pocl.icd, which names the library without a directory,
# so the loader's dlopen() finds it wherever the package root is. Programs
# link libOpenCL.so.1 and find the platform "Portable Computing Language".
# Also poclcc (compiles a kernel file to a PoCL binary ahead of time) and
# the built-in function library the kernels are linked with
# (share/pocl/kernel-<triple>-<cpu>.bc, LLVM bitcode).
#
# 7.2 is the release for LLVM 18 to 23; the llvm component is 22.1.8.
#
# How it is built for QNX:
#  - cross build as upstream supports it: LLVM_DIR is the target's LLVM
#    (staging), WITH_LLVM_CONFIG a host LLVM of the same release whose clang
#    compiles the built-in function library for the target triple
#    (pocl_host_llvm below builds those host tools once, from the llvm
#    component's source). The target's LLVM programs are named, not searched
#    for (patches/0004): the staging tree is not inside CMAKE_SYSROOT.
#  - the target CPU is fixed: x86-64 (the SSE2 baseline) and generic
#    (ARMv8-A with NEON) -- LLC_HOST_CPU. PoCL otherwise asks llc on the
#    build machine for its CPU, and at run time LLVM for the host CPU, which
#    it cannot identify on QNX/aarch64 (no /proc/cpuinfo); kernels compiled
#    for the baseline run on every target and in the QEMU test machines.
#    POCL_LLVM_CPU_NAME is not consulted with a fixed CPU.
#  - the device's description comes from the QNX system page: number of
#    CPUs, RAM, cache size, CPU name and speed (patches/0001); hwloc is not
#    used (not ported).
#  - the compiler runs on an 8 MiB stack (patches/0002): QNX threads have
#    256-512 KiB by default, Clang and LLVM need more.
#  - <net/netbyte.h> for the byte order functions (patches/0003); the data
#    directory lookup tolerates a library name without a directory
#    (patches/0005).
#  - the drivers are linked into libpocl (ENABLE_LOADABLE_DRIVERS=OFF: one
#    library, nothing to find at run time); LLVM and Clang are the shared
#    libraries of the llvm component, lld's ELF linker is linked statically
#    (the component has no shared lld); zlib and zstd are lld's.
#  - ENABLE_POCL_BUILDING=OFF: the "run from the build tree" mode would
#    compile the build machine's source and build paths into libpocl.
#  - left out: SPIR-V input (needs the SPIRV-LLVM-Translator, not ported),
#    the optional built-in kernels on onnxruntime, OpenCV and libjpeg-turbo
#    (found in staging otherwise: undeclared dependencies), vectorised math
#    through the sleef component's library on aarch64 (PoCL would compile
#    the library's staging path into the kernel link line), the remote,
#    Vulkan, CUDA and Level Zero drivers, tests and examples (they run on the
#    build machine). PoCL's copies of the OpenCL headers are not installed:
#    opencl-headers has them.
VERSION=7.2; GIT=https://github.com/pocl/pocl.git; REF=v7.2
DEPS="llvm zlib zstd opencl-headers opencl-icd-loader"
BUILDSYS=custom
POCL_LLVM=22.1.8      # the llvm component's version: host tools and target libraries must match
# Host LLVM tools of the same release as the target's libLLVM/libclang-cpp:
# pocl's kernel library (the OpenCL C built-in functions, as LLVM bitcode)
# is compiled at build time, for the target triple, by a clang that runs on
# the build machine, and the target's LLVM reads it at run time. Built once,
# from the llvm component's (QNX-patched) source so that it knows the same
# QNX triples, with the X86 and AArch64 back ends only and without
# optimisation (-O0 halves the build time of some 3000 files; these tools
# only compile the few hundred small files of the kernel library). Without
# optimisation GCC keeps a debug-only helper (printSpillReloadChain in
# MachineCopyPropagation.cpp) that nothing calls in a release build but that
# refers to MachineInstr::dump(), which a release build does not define:
# the symbol is given the address 0 at link time (--defsym).
pocl_host_llvm() {
  local H="$BUILD_DIR/host/pocl-llvm-$POCL_LLVM" L="$SRC_DIR/llvm"
  if [ ! -x "$H/bin/clang" ] || [ ! -x "$H/bin/llvm-link" ] || [ ! -x "$H/bin/llvm-config" ]; then
    [ -f "$L/llvm/CMakeLists.txt" ] || { echo "pocl: src/llvm is missing (scripts/qnxbuild llvm fetches and patches it)"; return 1; }
    env -u CC -u CXX cmake -S "$L/llvm" -B "$H" -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS=clang \
      -DCMAKE_C_FLAGS_RELEASE="-O0 -DNDEBUG" -DCMAKE_CXX_FLAGS_RELEASE="-O0 -DNDEBUG" \
      -DCMAKE_EXE_LINKER_FLAGS="-Wl,--defsym,_ZNK4llvm12MachineInstr4dumpEv=0" \
      -DLLVM_TARGETS_TO_BUILD="X86;AArch64" -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF \
      -DLLVM_ENABLE_ZLIB=OFF -DLLVM_ENABLE_ZSTD=OFF -DLLVM_ENABLE_LIBXML2=OFF -DLLVM_ENABLE_LIBEDIT=OFF -DLLVM_ENABLE_TERMINFO=OFF \
      -DCLANG_ENABLE_STATIC_ANALYZER=OFF -DLLVM_APPEND_VC_REV=OFF -DLLVM_ENABLE_BINDINGS=OFF &&
    ninja -C "$H" -j4 clang clang-resource-headers opt llc llvm-as llvm-dis llvm-link llvm-config || return 1
  fi
}
do_configure() {
  pocl_host_llvm || return 1
  local cpu=x86-64; [ "$QNX_CPU" = aarch64 ] && cpu=generic
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DWITH_LLVM_CONFIG="$BUILD_DIR/host/pocl-llvm-$POCL_LLVM/bin/llvm-config" -DLLVM_DIR="$STAGE$PREFIX/lib/cmake/llvm" \
    -DPOCL_TARGET_LLVM_BINDIR="$PREFIX/bin" -DSTATIC_LLVM=OFF -DLLC_HOST_CPU="$cpu" -DHOST_CPU_CACHELINE_SIZE=64 \
    -DENABLE_HOST_CPU_DEVICES=ON -DENABLE_LOADABLE_DRIVERS=OFF -DENABLE_HWLOC=OFF -DENABLE_SPIRV=OFF -DENABLE_HOST_CPU_VECTORIZE_SLEEF=OFF -DCMAKE_DISABLE_FIND_PACKAGE_onnxruntime=ON -DCMAKE_DISABLE_FIND_PACKAGE_LIBJPEG_TURBO=ON -DCMAKE_DISABLE_FIND_PACKAGE_OpenCV=ON \
    -DENABLE_ICD=ON -DPOCL_ICD_ABSOLUTE_PATH=OFF -DPOCL_INSTALL_ICD_VENDORDIR="$PREFIX/etc/OpenCL/vendors" \
    -DINSTALL_OPENCL_HEADERS=OFF -DENABLE_TESTS=OFF -DENABLE_EXAMPLES=OFF -DENABLE_POCLCC=ON -DENABLE_POCL_BUILDING=OFF
}
do_build() { cmake --build "$B" -j4; }
do_install() {
  DESTDIR="$STAGE" cmake --install "$B"
  install -Dm644 "$S/LICENSE" "$STAGE$PREFIX/share/licenses/pocl/LICENSE"
  install -m644 "$S/LICENSE.with.3rdparty" "$STAGE$PREFIX/share/licenses/pocl/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
