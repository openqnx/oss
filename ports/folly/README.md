# folly 2026.10.26.00

> Open-source C++ library developed and used at Facebook

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/facebook/folly.git (tag `v2026.10.26.00`) |
| Build system | CMake |
| QNX patches | 6 |
| Build dependencies | [`boost`](../boost/) [`fast-float`](../fast-float/) [`fmt`](../fmt/) [`gflags`](../gflags/) [`glog`](../glog/) [`libevent`](../libevent/) [`openssl`](../openssl/) [`zlib`](../zlib/) [`zstd`](../zstd/) [`lz4`](../lz4/) [`xz`](../xz/) [`bzip2`](../bzip2/) [`libsodium`](../libsodium/) |
| Run-time dependencies | [`boost`](../boost/) [`bzip2`](../bzip2/) [`fmt`](../fmt/) [`gflags`](../gflags/) [`glog`](../glog/) [`libevent`](../libevent/) [`libsodium`](../libsodium/) [`lz4`](../lz4/) [`openssl`](../openssl/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_folly`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `folly` (programs, shared libraries, run-time data) and `folly-dev` (headers, pkg-config/CMake files, static libraries; needs `folly`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`folly`](https://openqnx.github.io/oss/package/x86_64/folly.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/folly.html)) · [`folly-dev`](https://openqnx.github.io/oss/package/x86_64/folly-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/folly-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add folly` (dependencies come along); one version: `apk add folly=<version>` | `apk del folly` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install folly` (dependencies first) | `oss-get remove folly` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`folly-dev` the same way, by its own name (`apk add folly-dev`, `oss-get install folly-dev`, by hand with its own file and list `folly-dev.list`); remove it before `folly`, which it needs.

```sh
# on the QNX target, as root
apk add folly folly-dev
apk add qnx-test-folly && /usr/local/bin/qnx-port-tests/test_folly    # its on-target test, from tests-8.0-<arch>
apk del folly folly-dev qnx-test-folly
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/folly.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/folly.list
rm var/lib/manual/folly.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild folly` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (17 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild boost fast-float fmt gflags glog libevent openssl zlib zstd lz4 xz bzip2 libsodium
```

<details><summary>Full build chain (17)</summary>

[`boost`](../boost/) [`bzip2`](../bzip2/) [`fast-float`](../fast-float/) [`fmt`](../fmt/) [`gettext`](../gettext/) [`gflags`](../gflags/) [`glog`](../glog/) [`libevent`](../libevent/) [`libiconv`](../libiconv/) [`libsodium`](../libsodium/) [`libunistring`](../libunistring/) [`lz4`](../lz4/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v2026.10.26.00 --recurse-submodules --shallow-submodules \
    https://github.com/facebook/folly.git src/folly
```

### 3. Apply the QNX patches

6 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/folly < ports/folly/patches/0001-qnx-no-syscall.patch
patch -p1 -d src/folly < ports/folly/patches/0002-qnx-libc.patch
patch -p1 -d src/folly < ports/folly/patches/0003-qnx-farmhash.patch
patch -p1 -d src/folly < ports/folly/patches/0004-qnx-simple-math-sse2.patch
patch -p1 -d src/folly < ports/folly/patches/0005-qnx-cxxabi-supcxx.patch
patch -p1 -d src/folly < ports/folly/patches/0006-no-installed-certs-dir.patch
```

**[`0001-qnx-no-syscall.patch`](patches/0001-qnx-no-syscall.patch)** — Build without syscall(2) on QNX.

> QNX has neither <sys/syscall.h> nor syscall(2): system calls are message
> passes to the kernel and have no numbers. folly/portability/SysSyscall.h
> now skips the header on QNX and its linux_syscall() wrapper fails with
> ENOSYS, as it does on Windows and wasi; every caller already takes another
> path off Linux (they test kIsLinux). What still named Linux syscall
> numbers on every platform is guarded: getOSThreadIDSlow() uses QNX's
> gettid() (<sys/process.h>), and Subprocess's set*id() use local constants
> like the setres*id ones next to them. (SignalHandler.cpp's Linux-only pidfd
> path is in the next patch, with the rest of that file.)

Touches: `folly/portability/SysSyscall.h`, `folly/Subprocess.cpp`, `folly/system/ThreadId.cpp` · Upstream status: Pending

**[`0002-qnx-libc.patch`](patches/0002-qnx-libc.patch)** — Fill the QNX libc differences in folly/portability.

> Each change is guarded by __QNX__ or by the missing definition:
>  - IOV_MAX: QNX's <limits.h> does not define it; <sys/uio.h> has the same
>    limit as UIO_MAXIOV (folly/portability/IOVec.h).
>  - madvise(): QNX has only posix_madvise(), which returns the error number
>    instead of setting errno. SysMman.h maps MADV_NORMAL/SEQUENTIAL/RANDOM/
>    WILLNEED/DONTNEED to the POSIX_MADV_* values and defines madvise() over
>    posix_madvise() with errno set; advice QNX does not know (MADV_POPULATE_*,
>    MADV_COLLAPSE) fails with EINVAL, which the callers already handle.
>  - clearenv(): QNX's <stdlib.h> has it; folly's own replacement made the
>    call ambiguous (Stdlib.h/.cpp).
>  - recvmmsg()/sendmmsg(): QNX has both; folly's null function-pointer
>    stand-ins collided with the declarations (net/NetOps.cpp).
>  - SIGIO si_codes: QNX's <signal.h> has no POLL_IN..POLL_HUP; the fatal
>    signal reporter describes such codes generically there, and its
>    Linux-only pidfd path no longer names FOLLY_SYS_gettid, which QNX lacks.

Touches: `folly/debugging/symbolizer/SignalHandler.cpp`, `folly/net/NetOps.cpp`, `folly/portability/IOVec.h`, `folly/portability/Stdlib.cpp`, `folly/portability/Stdlib.h`, `folly/portability/SysMman.h` · Upstream status: Pending

**[`0003-qnx-farmhash.patch`](patches/0003-qnx-farmhash.patch)** — Build the bundled farmhash on QNX.

> Two QNX problems in folly/external/farmhash/farmhash.cpp:
>  - QNX has no <byteswap.h>; the GCC builtins are used, as the farmhash
>    component's own port does.
>  - farmhash defines a macro named x86_64 (1 on x86-64), and QNX's headers
>    build CPU-specific include paths from that bare token (<sys/ntohdr.h>:
>    x86_64/cpuinline.h). <algorithm>, included after the macro, then asked
>    for "1/cpuinline.h". It is included before the macro on QNX.

Touches: `folly/external/farmhash/farmhash.cpp` · Upstream status: Pending

**[`0004-qnx-simple-math-sse2.patch`](patches/0004-qnx-simple-math-sse2.patch)** — Keep SSE2 for LtHash's scalar fallback on QNX x86_64.

> MathOperation_Simple.cpp is compiled with -mno-avx -mno-avx2 -mno-sse2 to
> get a plain scalar variant. With x87 arithmetic GCC reports
> __FLT_EVAL_METHOD__ as -1 in C++, a value for which QNX's <math.h> declares
> no double_t or float_t, and libc++'s <cmath> fails ('double_t' has not been
> declared). On QNX the file keeps SSE2 (the x86-64 baseline) and loses only
> AVX: the code is the same scalar code, possibly vectorised by the compiler.

Touches: `folly/crypto/detail/CMakeLists.txt` · Upstream status: QNX-specific

**[`0005-qnx-cxxabi-supcxx.patch`](patches/0005-qnx-cxxabi-supcxx.patch)** — Use the libsupc++ exception ABI on QNX (libc++ over libsupc++).

> folly reads and builds std::exception_ptr objects through the C++ ABI
> runtime's internal structures, and picks the structures by C++ library:
> libstdc++ means GCC's libsupc++, libc++ means llvm's libc++abi (or libcxxrt
> on FreeBSD). QNX pairs libc++ with libsupc++: its libc++.so carries GCC's
> ABI runtime (__cxa_*, __cxxabiv1::__forced_unwind, ...), with no
> __cxa_increment_exception_refcount, so libfolly.so did not link
> ("undefined reference to __cxa_increment_exception_refcount"), and the
> libc++abi layout it would have read is not the one in memory. On QNX the
> libsupc++ definitions (__cxa_exception, __cxa_refcounted_exception) and
> code paths are used instead; std::current_exception() stays libc++'s.
>
> libc++'s std::type_info does not declare libsupc++'s virtual __do_catch,
> so exception_ptr_get_object_() calls it through a shim with libsupc++'s
> vtable order, as folly's FreeBSD (libcxxrt) path does.

Touches: `folly/lang/Exception.cpp` · Upstream status: Pending

**[`0006-no-installed-certs-dir.patch`](patches/0006-no-installed-certs-dir.patch)** — Do not export the source tree's FOLLY_CERTS_DIR.

> FOLLY_CERTS_DIR names the test certificates in the source tree, for the
> test utilities when folly's own tests are built. It was exported in the
> installed folly-targets.cmake too, so the package told consumers of
> Folly::folly_test_util to look for certificates at a path of the build
> machine. Keep the definition to the build tree ($<BUILD_INTERFACE:...>).

Touches: `CMakeLists.txt` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/folly -B build/$QNX_ARCH/folly -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DPACKAGE_VERSION=$VERSION \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TESTS=OFF \
      -DBUILD_BENCHMARKS=OFF \
      -DPYTHON_EXTENSIONS=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_DISABLE_FIND_PACKAGE_LibDwarf=ON \
      -DCMAKE_DISABLE_FIND_PACKAGE_Libiberty=ON \
      -DCMAKE_DISABLE_FIND_PACKAGE_LibAIO=ON \
      -DCMAKE_DISABLE_FIND_PACKAGE_LibUring=ON \
      -DCMAKE_DISABLE_FIND_PACKAGE_LibUnwind=ON \
      -DCMAKE_DISABLE_FIND_PACKAGE_Backtrace=ON \
      -DCMAKE_DISABLE_FIND_PACKAGE_Libevent=ON \
      -DCMAKE_DISABLE_FIND_PACKAGE_Snappy=ON \
      -DFOLLY_HAVE_UNALIGNED_ACCESS_EXITCODE=0 \
      -DFOLLY_HAVE_WEAK_SYMBOLS_EXITCODE=0 \
      -DFOLLY_HAVE_LINUX_VDSO_EXITCODE=1 \
      -DFOLLY_HAVE_WCHAR_SUPPORT_EXITCODE=0 \
      -DHAVE_VSNPRINTF_ERRORS_EXITCODE=1
cmake --build build/$QNX_ARCH/folly -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/folly
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- folly: Facebook's open source C++ library -- containers (F14 hash maps, small_vector, fbvector, ConcurrentHashMap), futures and executors, coroutines, string formatting and conversion (to/from), dynamic/JSON, synchronization primitives, IOBuf, async I/O on libevent, and more. folly tags a release every week; this is one of them.
- Not snappy: the snappy component is built without RTTI (as upstream builds it) and folly subclasses snappy::Source, whose typeinfo then does not exist (the same reason as in the rocksdb recipe). Optional dependencies not used: libdwarf/libiberty/libunwind/backtrace (folly's symbolizer and stack traces are written for glibc/Linux ELF introspection), libaio and liburing (Linux-only I/O). libevent is found by library name: the libevent component's CMake package file names /usr/local/lib directly instead of a path relative to itself, which does not exist on the build machine. Run-time probes a cross build cannot run (hazard 20), answered for QNX: unaligned loads work on x86_64 and on ARMv8-A (exit 0); an undefined weak symbol resolves to null as on any ELF system (0); there is no linux-vdso.so.1 (1); wcstol works (0); HAVE_VSNPRINTF_ERRORS is read by a test only (1). Shared libraries (upstream supports them, and recommends static for ABI reasons): the package then carries one libfolly.so instead of an archive that every consumer has to link with folly's dozen dependencies itself. The libraries' version, and so their soname, is folly's PACKAGE_VERSION, which in a source build defaults to "0.58.0-dev" (libfolly.so.0.58.0-dev). The release tag is the version: folly makes no ABI promise between weekly releases, so each one gets a soname of its own. (No double-conversion: this folly converts floating point with fast_float.)

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH folly`.

### 5. What it installs

1050 files under `/usr/local`: 20 shared-library entries, 1026 headers.

```
/usr/local/lib/libfolly.so
/usr/local/lib/libfolly.so.2026.10.26.00
/usr/local/lib/libfolly_benchmark_util.so
/usr/local/lib/libfolly_benchmark_util.so.2026.10.26.00
/usr/local/lib/libfolly_crypto_detail_math_operation_avx2_disable.so
/usr/local/lib/libfolly_crypto_detail_math_operation_avx2_disable.so.2026.10.26.00
/usr/local/lib/libfolly_crypto_detail_math_operation_sse2_disable.so
/usr/local/lib/libfolly_crypto_detail_math_operation_sse2_disable.so.2026.10.26.00
/usr/local/lib/libfolly_crypto_lt_hash_avx2.so
/usr/local/lib/libfolly_crypto_lt_hash_avx2.so.2026.10.26.00
/usr/local/lib/libfolly_crypto_lt_hash_sse2.so
/usr/local/lib/libfolly_crypto_lt_hash_sse2.so.2026.10.26.00
/usr/local/lib/libfolly_memcpy-use.so
/usr/local/lib/libfolly_memcpy-use.so.2026.10.26.00
/usr/local/lib/libfolly_memset-use.so
/usr/local/lib/libfolly_memset-use.so.2026.10.26.00
...
```

The complete list is `stage/<arch>/.manifest/folly` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa folly
ls dists/8.0/*/ | grep '^folly-'
```

This writes `folly-<ver>-r0.tar.gz` (run time) and `folly-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add folly
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/folly-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/folly-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_folly`](../../tests/src/) links folly the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-folly`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-folly
/usr/local/bin/qnx-port-tests/test_folly
```

```
== folly ==
  ok   ...
PASS folly
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_folly` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_folly`](../../tests/src/) -- a program linked against folly the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - fbstring
   - small_vector: inline, then on the heap
   - F14FastMap: 1000 entries
   - sorted_vector_map keeps order
   - to<int>, to<string>
   - double conversions (shortest round trip)
   - tryTo<int>(\"12x\") reports an error
   - to<uint8_t>(300) throws ConversionError (overflow)
   - split and join
   - stringPrintf, trimWhitespace
   - hexlify
   - parseJson
   - toJson with sorted keys
   - JSON round trip
   - Expected
   - Optional
   - Synchronized vector: 4 threads x 1000 pushes
   - ConcurrentHashMap: 2000 entries from 4 threads
   - via(pool).thenValue chain: 42
   - collectAll of 10 futures: sum of squares 385
   - exception caught by thenError
   - get() with a 50 ms timeout throws FutureTimeout
   - coroutine Task: ((1+2)+4)*6 = 42
   - coroutine scheduled on the pool
   - EventBase timer (30 ms) on its own thread
   - ... and 6 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | folly 2026.10.26.00-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/folly-2026.10.26.00-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | folly 2026.10.26.00-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/folly-2026.10.26.00-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2026.10.26.00**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add folly=<version>`), sources in `ported/folly/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates folly`; port one with `scripts/port-update folly`.

<details><summary>recipe.sh</summary>

```bash
# folly: Facebook's open source C++ library -- containers (F14 hash maps,
# small_vector, fbvector, ConcurrentHashMap), futures and executors,
# coroutines, string formatting and conversion (to/from), dynamic/JSON,
# synchronization primitives, IOBuf, async I/O on libevent, and more.
# folly tags a release every week; this is one of them.
VERSION=2026.10.26.00; GIT=https://github.com/facebook/folly.git; REF=v2026.10.26.00
DEPS="boost fast-float fmt gflags glog libevent openssl zlib zstd lz4 xz bzip2 libsodium"
# Not snappy: the snappy component is built without RTTI (as upstream builds
# it) and folly subclasses snappy::Source, whose typeinfo then does not exist
# (the same reason as in the rocksdb recipe).
# Optional dependencies not used: libdwarf/libiberty/libunwind/backtrace
# (folly's symbolizer and stack traces are written for glibc/Linux ELF
# introspection), libaio and liburing (Linux-only I/O). libevent is found by
# library name: the libevent component's CMake package file names
# /usr/local/lib directly instead of a path relative to itself, which does
# not exist on the build machine.
# Run-time probes a cross build cannot run (hazard 20), answered for QNX:
#  unaligned loads work on x86_64 and on ARMv8-A (exit 0); an undefined weak
#  symbol resolves to null as on any ELF system (0); there is no
#  linux-vdso.so.1 (1); wcstol works (0); HAVE_VSNPRINTF_ERRORS is read by a
#  test only (1).
# Shared libraries (upstream supports them, and recommends static for ABI
# reasons): the package then carries one libfolly.so instead of an archive
# that every consumer has to link with folly's dozen dependencies itself.
# The libraries' version, and so their soname, is folly's PACKAGE_VERSION,
# which in a source build defaults to "0.58.0-dev" (libfolly.so.0.58.0-dev).
# The release tag is the version: folly makes no ABI promise between weekly
# releases, so each one gets a soname of its own.
# (No double-conversion: this folly converts floating point with fast_float.)
CMAKE_ARGS_EXTRA=( -DPACKAGE_VERSION=$VERSION -DBUILD_SHARED_LIBS=ON -DBUILD_TESTS=OFF -DBUILD_BENCHMARKS=OFF -DPYTHON_EXTENSIONS=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_LibDwarf=ON -DCMAKE_DISABLE_FIND_PACKAGE_Libiberty=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_LibAIO=ON -DCMAKE_DISABLE_FIND_PACKAGE_LibUring=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_LibUnwind=ON -DCMAKE_DISABLE_FIND_PACKAGE_Backtrace=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_Libevent=ON -DCMAKE_DISABLE_FIND_PACKAGE_Snappy=ON
                   -DFOLLY_HAVE_UNALIGNED_ACCESS_EXITCODE=0 -DFOLLY_HAVE_WEAK_SYMBOLS_EXITCODE=0
                   -DFOLLY_HAVE_LINUX_VDSO_EXITCODE=1 -DFOLLY_HAVE_WCHAR_SUPPORT_EXITCODE=0
                   -DHAVE_VSNPRINTF_ERRORS_EXITCODE=1 )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
