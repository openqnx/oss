# javascriptcore 2.54.3

> WebKit's JavaScriptCore JavaScript engine (standalone JSCOnly port)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://webkitgtk.org/releases/webkitgtk-2.54.3.tar.xz |
| SHA-256 | `2779c1b46f88762e8400650271ba14452dc4e30b37059981552d8b562b7d0862` |
| Build system | CMake |
| QNX patches | 6 |
| Build dependencies | [`icu`](../icu/) |
| Run-time dependencies | [`icu`](../icu/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_javascriptcore`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `javascriptcore` (programs, shared libraries, run-time data) and `javascriptcore-dev` (headers, pkg-config/CMake files, static libraries; needs `javascriptcore`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`javascriptcore`](https://openqnx.github.io/oss/package/x86_64/javascriptcore.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/javascriptcore.html)) · [`javascriptcore-dev`](https://openqnx.github.io/oss/package/x86_64/javascriptcore-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/javascriptcore-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add javascriptcore` (dependencies come along); one version: `apk add javascriptcore=<version>` | `apk del javascriptcore` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install javascriptcore` (dependencies first) | `oss-get remove javascriptcore` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`javascriptcore-dev` the same way, by its own name (`apk add javascriptcore-dev`, `oss-get install javascriptcore-dev`, by hand with its own file and list `javascriptcore-dev.list`); remove it before `javascriptcore`, which it needs.

```sh
# on the QNX target, as root
apk add javascriptcore javascriptcore-dev
apk add qnx-test-javascriptcore && /usr/local/bin/qnx-port-tests/test_javascriptcore    # its on-target test, from tests-8.0-<arch>
apk del javascriptcore javascriptcore-dev qnx-test-javascriptcore
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/javascriptcore.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/javascriptcore.list
rm var/lib/manual/javascriptcore.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild javascriptcore` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild icu
```

### 2. Get the upstream source

```bash
URL=https://webkitgtk.org/releases/webkitgtk-2.54.3.tar.xz
mkdir -p dist src/javascriptcore
curl -fL -o dist/$(basename $URL) $URL
echo "2779c1b46f88762e8400650271ba14452dc4e30b37059981552d8b562b7d0862  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/javascriptcore --strip-components=1
```

### 3. Apply the QNX patches

6 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/javascriptcore < ports/javascriptcore/patches/0003-jsconly-qnx-memorypressure.patch
patch -p1 -d src/javascriptcore < ports/javascriptcore/patches/0010-qnx-madvise-posix-madvise.patch
patch -p1 -d src/javascriptcore < ports/javascriptcore/patches/0011-qnx-wtf-memory-status.patch
patch -p1 -d src/javascriptcore < ports/javascriptcore/patches/0012-jsc-float16-isnan-libcxx.patch
patch -p1 -d src/javascriptcore < ports/javascriptcore/patches/0013-wtf-qnx-timerfd-extern-c.patch
patch -p1 -d src/javascriptcore < ports/javascriptcore/patches/0015-wtf-switchon-visitonevariant-clang-only.patch
```

**[`0003-jsconly-qnx-memorypressure.patch`](patches/0003-jsconly-qnx-memorypressure.patch)** — JSCOnly: select the Unix memory-pressure handler on QNX.

> Linking fails with:
>
>     MemoryPressureHandler.cpp: undefined reference to
>         WTF::MemoryPressureHandler::holdOffTimerFired()
>
> MemoryPressureHandler.h declares m_holdOffTimer and holdOffTimerFired() for
> "OS(LINUX) || OS(FREEBSD) || OS(QNX)", and unix/MemoryPressureHandlerUnix.cpp
> defines them -- it already has an OS(QNX) branch that reads the resident set
> size with devctl(DCMD_PROC_ASINFO) on /proc/self/ctl.  WTF knows about QNX; the
> JSCOnly platform list simply had no QNX case, so the build fell through to
> generic/MemoryPressureHandlerGeneric.cpp, which does not define the members the
> header declared.

Touches: `Source/WTF/wtf/PlatformJSCOnly.cmake` · Upstream status: QNX-specific

**[`0010-qnx-madvise-posix-madvise.patch`](patches/0010-qnx-madvise-posix-madvise.patch)** — bmalloc, WTF: madvise() through posix_madvise() on QNX.

> VMAllocate.h:327: error: 'madvise' was not declared in this scope
>     VMAllocate.h:327: error: 'MADV_DONTNEED' was not declared in this scope
>     VMAllocate.h:344: error: 'MADV_NORMAL' was not declared in this scope
>
> QNX's libc has no madvise() and no MADV_* constants; <sys/mman.h> offers the
> POSIX form, posix_madvise() with POSIX_MADV_NORMAL/WILLNEED/DONTNEED, which
> returns the error number instead of -1/errno.  bmalloc's VMAllocate.h (included
> even with USE_SYSTEM_MALLOC) and WTF's OSAllocatorPOSIX.cpp -- whose QNX
> branches upstream already call madvise(MADV_WILLNEED/MADV_DONTNEED) -- get a
> small static madvise() that forwards to posix_madvise() and sets errno, so the
> callers' "== -1 && errno == EAGAIN" loops are unchanged.  The block is the same
> in both files and guarded by !defined(MADV_NORMAL), so a unified source that
> includes both defines it once.  DONTNEED is advice here, as it is for every
> non-Linux caller: none of these paths rely on the pages reading back as zero
> (vmZeroAndPurge() re-maps with mmap(MAP_FIXED) on everything but Linux).

Touches: `Source/bmalloc/bmalloc/VMAllocate.h`, `Source/WTF/wtf/posix/OSAllocatorPOSIX.cpp` · Upstream status: Pending

**[`0011-qnx-wtf-memory-status.patch`](patches/0011-qnx-wtf-memory-status.patch)** — WTF: memoryStatus() on QNX.

> generic/MemoryFootprintGeneric.cpp:36: error: 'memoryStatus' was not
>         declared in this scope
>
> Both the GTK and the JSCOnly platform lists build
> generic/MemoryFootprintGeneric.cpp for QNX (alongside
> unix/MemoryPressureHandlerUnix.cpp), and since 2.46 that file computes the
> footprint through WTF::memoryStatus() -- which AvailableMemory.h declares only
> for iOS, Linux and FreeBSD.  Add QNX to those guards and give memoryStatus() a
> QNX branch: the resident set size from devctl(DCMD_PROC_ASINFO) on
> /proc/self/ctl, exactly how MemoryPressureHandlerUnix.cpp already measures the
> process on QNX; the percentage uses availableMemory() (sysconf
> _SC_PHYS_PAGES on QNX) like the other platforms.

Touches: `Source/WTF/wtf/AvailableMemory.h`, `Source/WTF/wtf/AvailableMemory.cpp` · Upstream status: Pending

**[`0012-jsc-float16-isnan-libcxx.patch`](patches/0012-jsc-float16-isnan-libcxx.patch)** — JSC: call std::isnan() on a double for Float16 typed arrays.

> JSGenericTypedArrayViewPrototypeFunctions.h:532: error: no matching
>         function for call to 'isnan(std::optional<WTF::Float16>::value_type&)'
>     c++/v1/__math/traits.h:111: error: no type named 'type' in
>         'struct std::enable_if<false, int>'
>
> QNX's C++ library is libc++, whose std::isnan() is a template restricted to
> arithmetic types (plus float/double/long double overloads that the class does
> not match exactly), so WTF::Float16 -- a class with an implicit conversion to
> double -- does not deduce.  libstdc++'s plain overloads take it through the
> conversion, which is why Linux builds pass.  includes() for Float16Array then
> fails to compile; convert explicitly, as the branch for the other float element
> sizes right below already does.  Same value, same result.

Touches: `Source/JavaScriptCore/runtime/JSGenericTypedArrayViewPrototypeFunctions.h` · Upstream status: Pending

**[`0013-wtf-qnx-timerfd-extern-c.patch`](patches/0013-wtf-qnx-timerfd-extern-c.patch)** — WTF: include QNX's <sys/timerfd.h> as a C header.

> RunLoopGLib.cpp: undefined reference to `timerfd_create(int, int)'
>     RunLoopGLib.cpp: undefined reference to `timerfd_settime(int, int,
>         itimerspec const*, itimerspec*)'
>
> The GLib run loop uses timerfd when CMake finds timerfd_create() (HAVE_TIMERFD),
> and on QNX it does: libtimerfd, linked by the toolchain's default libraries.
> But its <sys/timerfd.h> declares the functions without an extern "C" block, so
> C++ code references the mangled names and the link of libWTF fails.  Wrap the
> include in extern "C" on QNX.  (JSCOnly uses the generic run loop and does not
> build this file; the patch is carried in both ports to keep the shared tree the
> same.)

Touches: `Source/WTF/wtf/glib/RunLoopGLib.cpp` · Upstream status: Inappropriate [QNX system header lacks C++ guards]

**[`0015-wtf-switchon-visitonevariant-clang-only.patch`](patches/0015-wtf-switchon-visitonevariant-clang-only.patch)** — WTF: use the switch-based switchOn() for libc++ only with Clang.

> StyleTabSize.h:66: error: call to non-'constexpr' function
>         'decltype (WTF::visitOneVariant(...)) WTF::switchOn(V&&, F&& ...)'
>     (in 'constexpr bool WebCore::Style::TabSize::isZero() const')
>
> When the C++ library is libc++, StdLibExtras.h replaces switchOn() with a
> switch-based visitOneVariant() to work around code-size problems of libc++'s
> visit (webkit.org/b/279498).  That version is not constexpr; Clang in C++23
> mode accepts constexpr functions that call it (P2448), GCC 12 does not.  QNX's
> toolchain is exactly that pair -- GCC with libc++ -- so every constexpr caller
> in WebCore (TabSize::isZero() and the other style value types) fails.  Take the
> size workaround only with Clang; GCC + libc++ uses the generic constexpr
> switchOn() over WTF::visit, as GCC + libstdc++ already does.

Touches: `Source/WTF/wtf/StdLibExtras.h` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/javascriptcore; B=build/$QNX_ARCH/javascriptcore; mkdir -p $B
```

```bash
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  install -d "$STAGE$PREFIX/include/JavaScriptCore" "$STAGE$PREFIX/lib/pkgconfig"
  install -m644 "$B"/JavaScriptCore/Headers/JavaScriptCore/*.h \
                "$STAGE$PREFIX/include/JavaScriptCore/" || return 1
  cat > "$STAGE$PREFIX/lib/pkgconfig/javascriptcore.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include
Name: JavaScriptCore
Description: WebKit's JavaScript engine (JSCOnly port, CLoop interpreter)
Version: $VERSION
Libs: -L\${libdir} -lJavaScriptCore
Cflags: -I\${includedir}
PC
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- WebKit's own release tarball; the JSCOnly port builds JavaScriptCore, WTF and bmalloc without any of the web engine or platform integration.  Worth having on its own -- a scripting engine for HMI code -- and it settles whether this codebase compiles for QNX at all before the GTK port's much longer build.
- -DENABLE_JIT=OFF / -DENABLE_C_LOOP=ON: the interpreter, not the JIT.  JSC's JIT needs W^X handling and signal-based OSR that are written per-OS; the CLoop interpreter is the portable path and is what a first port should prove. ENABLE_FTL_JIT and the sampling profiler follow the JIT off. ENABLE_STATIC_JSC=OFF keeps the shared library that consumers link.
- (No CMAKE_CXX_STANDARD here: OptionsCommon.cmake sets C++23 itself, and WebKitCommon.cmake requires GCC >= 12.2 -- exactly the SDP's compiler.)
- JSCOnly installs the library and nothing else: the public C API headers are generated into the build tree and left there, so a consumer has no way to include <JavaScriptCore/JavaScript.h>.  Install them, and write the .pc file the port does not provide either.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH javascriptcore`.

### 5. What it installs

12 files under `/usr/local`: 3 shared-library entries, 8 headers.

```
/usr/local/lib/libJavaScriptCore.so
/usr/local/lib/libJavaScriptCore.so.1
/usr/local/lib/libJavaScriptCore.so.1.0.0
```

The complete list is `stage/<arch>/.manifest/javascriptcore` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa javascriptcore
ls dists/8.0/*/ | grep '^javascriptcore-'
```

This writes `javascriptcore-<ver>-r0.tar.gz` (run time) and `javascriptcore-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add javascriptcore
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/javascriptcore-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/javascriptcore-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_javascriptcore`](../../tests/src/) links javascriptcore the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-javascriptcore`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-javascriptcore
/usr/local/bin/qnx-port-tests/test_javascriptcore
```

```
== javascriptcore ==
  ok   ...
PASS javascriptcore
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_javascriptcore` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_javascriptcore`](../../tests/src/) -- a program linked against javascriptcore the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - JSContextGroupCreate
   - JSGlobalContextCreateInGroup
   - arithmetic and precedence
   - array sort with a closure
   - recursion (fib 20)
   - string and parseInt
   - JSON.parse
   - Math through libm
   - ES6 Map
   - a thrown Error reaches the embedder
   - the exception carries its message
   - JSON.stringify
   - the context still works after an explicit GC

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | javascriptcore 2.44.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/javascriptcore-2.44.4-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | javascriptcore 2.44.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/javascriptcore-2.44.4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.54.3**. Kept from earlier: `2.44.4` ([recipe and patches](versions/2.44.4/)). Every version stays available: packages in `dists/8.0/<arch>/` (`apk add javascriptcore=<version>`), sources in `ported/javascriptcore/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates javascriptcore`; port one with `scripts/port-update javascriptcore`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.54.3
# WebKit's own release tarball; the JSCOnly port builds JavaScriptCore, WTF and
# bmalloc without any of the web engine or platform integration.  Worth having
# on its own -- a scripting engine for HMI code -- and it settles whether this
# codebase compiles for QNX at all before the GTK port's much longer build.
TARBALL=https://webkitgtk.org/releases/webkitgtk-2.54.3.tar.xz
SHA256=2779c1b46f88762e8400650271ba14452dc4e30b37059981552d8b562b7d0862
DEPS="icu"
BUILDSYS=cmake
# -DENABLE_JIT=OFF / -DENABLE_C_LOOP=ON: the interpreter, not the JIT.  JSC's
# JIT needs W^X handling and signal-based OSR that are written per-OS; the
# CLoop interpreter is the portable path and is what a first port should prove.
# ENABLE_FTL_JIT and the sampling profiler follow the JIT off.
# ENABLE_STATIC_JSC=OFF keeps the shared library that consumers link.
CMAKE_ARGS_EXTRA=( -DPORT=JSCOnly -DENABLE_STATIC_JSC=OFF
                   -DENABLE_JIT=OFF -DENABLE_C_LOOP=ON -DENABLE_FTL_JIT=OFF
                   -DENABLE_SAMPLING_PROFILER=OFF
                   -DENABLE_WEBASSEMBLY=OFF
                   -DENABLE_API_TESTS=OFF -DENABLE_TOOLS=OFF
                   -DUSE_SYSTEM_MALLOC=ON
                   # gtest's death-test regex uses regcomp/regexec/regfree, which on
                   # QNX live in libregex, not libc.  OptionsJSCOnly.cmake forces
                   # ENABLE_API_TESTS back on for every non-Windows port, so gtest is
                   # built whatever -DENABLE_API_TESTS says.  The list repeats the
                   # toolchain's default because a -D replaces it rather than adding.
                   -DCMAKE_CXX_STANDARD_LIBRARIES="-lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify -lregex"
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
# (No CMAKE_CXX_STANDARD here: OptionsCommon.cmake sets C++23 itself, and
# WebKitCommon.cmake requires GCC >= 12.2 -- exactly the SDP's compiler.)

# JSCOnly installs the library and nothing else: the public C API headers are
# generated into the build tree and left there, so a consumer has no way to
# include <JavaScriptCore/JavaScript.h>.  Install them, and write the .pc file
# the port does not provide either.
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  install -d "$STAGE$PREFIX/include/JavaScriptCore" "$STAGE$PREFIX/lib/pkgconfig"
  install -m644 "$B"/JavaScriptCore/Headers/JavaScriptCore/*.h \
                "$STAGE$PREFIX/include/JavaScriptCore/" || return 1
  cat > "$STAGE$PREFIX/lib/pkgconfig/javascriptcore.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include
Name: JavaScriptCore
Description: WebKit's JavaScript engine (JSCOnly port, CLoop interpreter)
Version: $VERSION
Libs: -L\${libdir} -lJavaScriptCore
Cflags: -I\${includedir}
PC
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
