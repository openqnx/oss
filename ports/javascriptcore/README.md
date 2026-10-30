# javascriptcore 2.44.4

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://webkitgtk.org/releases/webkitgtk-2.44.4.tar.xz |
| SHA-256 | `2ce4ec1b78413035037aba8326b31ed72696626b7bea7bace5e46ac0d8cbe796` |
| Build system | CMake |
| QNX patches | 4 |
| Build dependencies | [`icu`](../icu/) |
| Run-time dependencies | [`icu`](../icu/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_javascriptcore`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add javascriptcore
apk add javascriptcore-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-javascriptcore && /usr/local/bin/qnx-port-tests/test_javascriptcore    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install javascriptcore`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
URL=https://webkitgtk.org/releases/webkitgtk-2.44.4.tar.xz
mkdir -p dist src/javascriptcore
curl -fL -o dist/$(basename $URL) $URL
echo "2ce4ec1b78413035037aba8326b31ed72696626b7bea7bace5e46ac0d8cbe796  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/javascriptcore --strip-components=1
```

### 3. Apply the QNX patches

4 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/javascriptcore < ports/javascriptcore/patches/0001-cmake4-quote-framework-vars.patch
patch -p1 -d src/javascriptcore < ports/javascriptcore/patches/0002-icu76-cplusplus-header-api.patch
patch -p1 -d src/javascriptcore < ports/javascriptcore/patches/0003-jsconly-qnx-memorypressure.patch
patch -p1 -d src/javascriptcore < ports/javascriptcore/patches/0004-qnx-structure-heap-128mb.patch
```

**[`0001-cmake4-quote-framework-vars.patch`](patches/0001-cmake4-quote-framework-vars.patch)** — Quote the framework variables in _WEBKIT_TARGET_LINK_FRAMEWORK.

> Configuring with CMake 4 fails before any compilation:
>
>     CMake Error at Source/cmake/WebKitMacros.cmake:280 (if):
>       if given arguments:
>         "(" "NOT" "_linked_into" ")" "OR" "(" "WTF" "STREQUAL" ")" "OR" ...
>       Unknown arguments specified
>
> When a framework has no LINKED_INTO property, ${_linked_into} expands to
> nothing and STREQUAL loses its right-hand operand.  Older CMake accepted the
> malformed expression; 4.x does not.  Quoting both expansions makes the
> comparison well-formed for every CMake version, and changes nothing about what
> it decides.  Not QNX-specific.

Touches: `Source/cmake/WebKitMacros.cmake` · Upstream status: Pending

**[`0002-icu76-cplusplus-header-api.patch`](patches/0002-icu76-cplusplus-header-api.patch)** — Turn off ICU 76's second C++ API switch as well.

> WTF/Platform.h sets U_SHOW_CPLUSPLUS_API 0 because WebKit uses only ICU's C
> API.  ICU 76 introduced U_SHOW_CPLUSPLUS_HEADER_API, defaulting to 1, and moved
> some declarations behind "U_SHOW_CPLUSPLUS_API || U_SHOW_CPLUSPLUS_HEADER_API".
> Building against ICU 76 therefore compiles a std::u16string_view iterator in
> uset.h whose helper types the first switch has already suppressed:
>
>     unicode/uset.h:1660:21: error: 'ConstChar16Ptr' was not declared in this scope
>     unicode/uset.h:1763:5:  error: 'UnicodeString' does not name a type
>
> Setting both to 0 is what the ICU 76 release notes prescribe for C-only
> consumers.  Not QNX-specific: any WebKit 2.44 build against ICU >= 76 hits it.

Touches: `Source/WTF/wtf/Platform.h` · Upstream status: Pending

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

**[`0004-qnx-structure-heap-128mb.patch`](patches/0004-qnx-structure-heap-128mb.patch)** — Use a 128 MB structure heap on QNX.

> JSC reserves its structure heap with tryReserveUncommittedAligned(size,
> structureHeapAddressSize).  StructureMemoryManager halves the size up to eight
> times when the reservation fails, but the *alignment* argument stays at
> structureHeapAddressSize -- so with the 4 GB default every attempt needs up to
> 8 GB of contiguous address space.  On a QNX target with 1 GB of RAM all eight
> fail and the RELEASE_ASSERT aborts inside JSContextGroupCreate(), before any
> script runs.  It passes on an 8 GB board, so the failure looks like an
> architecture difference and is really an address-space one.
>
> 128 MB is the value PLAYSTATION already uses for the same reason.  StructureID
> derives its masks from this constant, so nothing else needs changing.

Touches: `Source/JavaScriptCore/runtime/StructureID.h` · Upstream status: QNX-specific

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

[`test_javascriptcore`](../../tests/src/) links javascriptcore the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-javascriptcore`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

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

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | javascriptcore 2.44.4-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | javascriptcore 2.44.4-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **2.44.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add javascriptcore=<version>`), sources in `ported/javascriptcore/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates javascriptcore`; port one with `scripts/port-update javascriptcore`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.44.4
# WebKit's own release tarball; the JSCOnly port builds JavaScriptCore, WTF and
# bmalloc without any of the web engine or platform integration.  Worth having
# on its own -- a scripting engine for HMI code -- and it settles whether this
# codebase compiles for QNX at all before the GTK port's much longer build.
TARBALL=https://webkitgtk.org/releases/webkitgtk-2.44.4.tar.xz
SHA256=2ce4ec1b78413035037aba8326b31ed72696626b7bea7bace5e46ac0d8cbe796
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
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                   -DCMAKE_CXX_STANDARD=20 )

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
