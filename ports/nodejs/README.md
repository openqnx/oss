# nodejs 24.14.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/nodejs/node.git (tag `v24.14.1`) |
| Build system | custom (see recipe) |
| QNX patches | 9 |
| Build dependencies | [`libuv`](../libuv/) [`ada`](../ada/) [`brotli`](../brotli/) [`zlib`](../zlib/) [`openssl`](../openssl/) [`c-ares`](../c-ares/) [`nghttp2`](../nghttp2/) [`simdjson`](../simdjson/) [`simdutf`](../simdutf/) [`sqlite`](../sqlite/) [`zstd`](../zstd/) [`icu`](../icu/) |
| Run-time dependencies | [`ada`](../ada/) [`brotli`](../brotli/) [`c-ares`](../c-ares/) [`icu`](../icu/) [`libuv`](../libuv/) [`nghttp2`](../nghttp2/) [`openssl`](../openssl/) [`simdjson`](../simdjson/) [`simdutf`](../simdutf/) [`sqlite`](../sqlite/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add nodejs
apk add nodejs-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install nodejs`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild nodejs` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (12 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libuv ada brotli zlib openssl c-ares nghttp2 simdjson simdutf sqlite zstd icu
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v24.14.1 --recurse-submodules --shallow-submodules \
    https://github.com/nodejs/node.git src/nodejs
```

### 3. Apply the QNX patches

9 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/nodejs < ports/nodejs/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/nodejs < ports/nodejs/patches/0002-aports-remove-unused-openssl-config.patch
patch -p1 -d src/nodejs < ports/nodejs/patches/0003-aports-v8-no-static-zlib.patch
patch -p1 -d src/nodejs < ports/nodejs/patches/0004-aports-v8-int64-lowering-reducer.patch
patch -p1 -d src/nodejs < ports/nodejs/patches/0005-aports-node_sea.patch
patch -p1 -d src/nodejs < ports/nodejs/patches/0006-aports-node_snapshotable.patch
patch -p1 -d src/nodejs < ports/nodejs/patches/0007-v8config-target-os-fallback-value.patch
patch -p1 -d src/nodejs < ports/nodejs/patches/0008-v8-wasm-objects-include-v8config-first.patch
patch -p1 -d src/nodejs < ports/nodejs/patches/0009-v8-gyp-host-trap-handler-for-qnx.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — initial qnx support

> Imported from qnx-ports (nodejs/node v24.14.1...qnx-v24.14.1); original authorship kept.
>
>
>
> ---
>  common.gypi                                 | 17 ++++++++++++++++-
>  configure.py                                |  2 +-
>  deps/v8/src/base/platform/memory.h          |  5 ++++-
>  deps/v8/src/base/platform/platform-posix.cc |  8 ++++++--
>  deps/v8/src/base/platform/platform-qnx.cc   |  2 +-
>  src/debug_utils.cc                          |  2 +-
>  src/node_credentials.cc                     |  8 ++++----
>  tools/v8_gypfiles/v8.gyp                    | 10 +++++-----
>  8 files changed, 38 insertions(+), 16 deletions(-)

Touches: `common.gypi`, `configure.py`, `deps/v8/src/base/platform/memory.h`, `deps/v8/src/base/platform/platform-posix.cc`, `deps/v8/src/base/platform/platform-qnx.cc`, `src/debug_utils.cc` … · Upstream status: QNX-specific (from qnx-ports)

**[`0002-aports-remove-unused-openssl-config.patch`](patches/0002-aports-remove-unused-openssl-config.patch)** — QNX build fix from QNX's aports (extra/nodejs/remove-unused-openssl-config.patch).

> Taken from qnx-ports/aports extra/nodejs, the APKBUILD the QNX Everywhere image's
> package is built from.
>
>
> Patch-Source: https://src.fedoraproject.org/rpms/nodejs24/blob/rawhide/f/0001-Remove-unused-OpenSSL-config.patch
> --
> Remove unused OpenSSL config
>
> The build process will try to create these config files, even when
> using the system OpenSSL and will thus fail since we strip this path
> from the tarball.
>
> Signed-off-by: Stephen Gallagher <sgallagh@redhat.com>
> Signed-off-by: rpm-build <rpm-build>
> ---
>  node.gyp | 17 -----------------
>  1 file changed, 17 deletions(-)

Touches: `node.gyp` · Upstream status: QNX-specific

**[`0003-aports-v8-no-static-zlib.patch`](patches/0003-aports-v8-no-static-zlib.patch)** — QNX build fix from QNX's aports (extra/nodejs/v8-no-static-zlib.patch).

> Taken from qnx-ports/aports extra/nodejs, the APKBUILD the QNX Everywhere image's
> package is built from.
>
>
> Patch-Source: https://sources.debian.org/src/nodejs/20.18.0%2Bdfsg-2/debian/patches/deps/v8-no-static-zlib.patch
> --
> Description: compile v8 without zlib static lib
>  it should have a minor performance impact
> Author: Jérémy Lal <kapouer@melix.org>
> Last-Update: 2023-12-16
> Forwarded: https://github.com/nodejs/node/issues/33848

Touches: `tools/v8_gypfiles/features.gypi`, `tools/v8_gypfiles/v8.gyp` · Upstream status: QNX-specific

**[`0004-aports-v8-int64-lowering-reducer.patch`](patches/0004-aports-v8-int64-lowering-reducer.patch)** — QNX build fix from QNX's aports (extra/nodejs/v8-int64-lowering-reducer.patch).

> Taken from qnx-ports/aports extra/nodejs, the APKBUILD the QNX Everywhere image's
> package is built from.
>
>
> Fix build on 32-bit arches
>
>     ../../deps/v8/src/compiler/turboshaft/int64-lowering-reducer.h:640:15: error: invalid use of 'struct v8::internal::compiler::turboshaft::Tuple<v8::internal::compiler::turboshaft::WordWithBits<32>, v8::internal::compiler::turboshaft::WordWithBits<32> >' [-Wtemplate-body]
>       640 |     return __ Tuple<Word32, Word32>(result, __ Word32Constant(0));
>           |               ^~~~~~~~~~~~~~~~~~~~~
>
> See https://gitlab.alpinelinux.org/alpine/aports/-/merge_requests/88748#note_560642

Touches: `deps/v8/src/compiler/turboshaft/int64-lowering-reducer.h` · Upstream status: QNX-specific

**[`0005-aports-node_sea.patch`](patches/0005-aports-node_sea.patch)** — QNX build fix from QNX's aports (extra/nodejs/node_sea.patch).

> Taken from qnx-ports/aports extra/nodejs, the APKBUILD the QNX Everywhere image's
> package is built from.
>
>
> Bind string as string_view via temporary variable
>
> Cast string to string_view using a temporary variable to allow pass-by-reference to prevent error: cannot bind non-const lvalue reference of type 'std::basic_string_view&' to an rvalue of type 'std::__cxx11::basic_string::__sv_type' {aka 'std::basic_string_view'}

Touches: `src/node_sea.cc` · Upstream status: QNX-specific

**[`0006-aports-node_snapshotable.patch`](patches/0006-aports-node_snapshotable.patch)** — QNX build fix from QNX's aports (extra/nodejs/node_snapshotable.patch).

> Taken from qnx-ports/aports extra/nodejs, the APKBUILD the QNX Everywhere image's
> package is built from.
>
>
> Initialize as string_view
>
> Initialize as string_view to allow pass-by-reference to prevent error: cannot bind non-const lvalue reference of type 'std::basic_string_view<char>&' to an rvalue of type 'std::__cxx11::basic_string<char>::__sv_type' {aka 'std::basic_string_view<char>'}

Touches: `src/node_snapshotable.cc` · Upstream status: QNX-specific

**[`0007-v8config-target-os-fallback-value.patch`](patches/0007-v8config-target-os-fallback-value.patch)** — V8: give the derived V8_TARGET_OS_* macros the value 1.

> When the build does not set V8_HAVE_TARGET_OS, v8config.h derives the
> target OS from the host OS (V8_OS_LINUX -> V8_TARGET_OS_LINUX) and defines
> the macro empty. V8 tests it both ways -- #if defined(...) and #if
> V8_TARGET_OS_LINUX (wasm-objects.cc, std-object-sizes.h, d8.cc) -- and the
> second form needs a value: "#if with no expression". The build system's own
> -DV8_TARGET_OS_LINUX means 1; do the same. This is the path the host half
> of a cross build to QNX takes, as no V8_TARGET_OS_* exists for QNX.

Touches: `deps/v8/include/v8config.h` · Upstream status: Pending

**[`0008-v8-wasm-objects-include-v8config-first.patch`](patches/0008-v8-wasm-objects-include-v8config-first.patch)** — V8: include v8config.h before testing V8_TARGET_OS_LINUX in wasm-objects.cc.

> wasm-objects.cc includes <sys/mman.h> and <sys/stat.h> under
> #if V8_TARGET_OS_LINUX at the top of the file, before any V8 header. When
> the build passes no target OS -- the host half of a cross build to QNX --
> v8config.h defines the macro, later: the test at the top sees it undefined,
> the headers are skipped, and the Linux code further down then fails
> ("memfd_create was not declared").

Touches: `deps/v8/src/wasm/wasm-objects.cc` · Upstream status: Pending

**[`0009-v8-gyp-host-trap-handler-for-qnx.patch`](patches/0009-v8-gyp-host-trap-handler-for-qnx.patch)** — V8 gyp: trap-handler sources for the host half of a QNX build.

> V8's trap-handler.h decides by the OS it is compiled on whether the wasm trap
> handler exists (Linux on x64; the arm64 simulator on x64 Linux). The gyp
> files add the matching sources by gyp's OS, which is the target's -- qnx --
> for the host toolset too. The host build of V8 for a QNX target on a Linux
> machine therefore compiled code that calls the trap handler without its
> sources, and mksnapshot failed to link (undefined trap_handler::TryHandleSignal,
> v8_internal_simulator_ProbeMemory). Add them for that host toolset.

Touches: `tools/v8_gypfiles/v8.gyp` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/nodejs; B=build/$QNX_ARCH/nodejs; mkdir -p $B
```

```bash
do_configure() {
  local R; R=$(_node_sysroot) || return 1
  local HL="$BUILD_DIR/host/node-$VERSION-libs"
  _node_hostlibs "$HL" || return 1
  # Node builds in the source tree (out/); the other architecture's objects
  # must not be reused.
  rm -rf "$S/out" && cd "$S" && PKG_CONFIG_SYSROOT_DIR="$R" GYP_DEFINES="host_os=linux" CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc" CXX="$QNX_HOST/usr/bin/$QNX_TRIPLE-g++" CC_host=gcc CXX_host=g++ \
  CFLAGS="-D_QNX_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DOPENSSL_NO_ENGINE -idirafter $STAGE$PREFIX/include" \
  CXXFLAGS="-D_QNX_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DOPENSSL_NO_ENGINE -idirafter $STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib" PKG_CONFIG_LIBDIR="$PKG_CONFIG_LIBDIR" \
  python3 configure.py --prefix="$PREFIX" --dest-os=qnx --dest-cpu="$([ "$QNX_CPU" = aarch64 ] && echo arm64 || echo x64)" \
    --cross-compiling --use-prefix-to-find-headers --shared-libuv --shared-ada --shared-brotli --shared-zlib \
    --shared-openssl --shared-cares --shared-nghttp2 --shared-simdjson --shared-simdutf --shared-sqlite --shared-zstd \
    --openssl-use-def-ca-store --with-intl=system-icu --without-corepack --without-npm \
    --shared-libuv-includes="$R$PREFIX/include" --shared-libuv-libpath="$STAGE$PREFIX/lib"
}
```

```bash
do_build() { local HL="$BUILD_DIR/host/node-$VERSION-libs"
  LDFLAGS_host="-L$HL/lib -Wl,-rpath,$HL/lib -Wl,--disable-new-dtags" make -C "$S/out" BUILDTYPE=Release -j"$(nproc)" $NODE_HOST_TOOLS && \
  LDFLAGS_host="-L$HL/lib -Wl,-rpath,$HL/lib -Wl,--disable-new-dtags" make -C "$S/out" BUILDTYPE=Release -j"$(nproc)"; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$S/out/Release/node" "$STAGE$PREFIX/bin/" && \
  cd "$S" && python3 tools/install.py install --dest-dir "$STAGE" --prefix "$PREFIX" --headers-only; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Node.js 24 with qnx-ports' QNX support. patches/0001: the qnx-ports/node fork's QNX commit (libuv, V8 and the build files learn QNX); 0002-0006: QNX's aports changes the QNX Everywhere image builds with. The shared libraries of this tree are used instead of the bundled copies, as aports does. Cross build: V8's build runs generator programs (mksnapshot, torque, ...) on the build host, so configure gets the host compiler for those (CC_host) and the QNX compiler for the target.
- V8's gyp files test host_os for the host toolset (mksnapshot, torque run on the build machine). toolchain.gypi gives it a default of OS, but in a cross build that default is not in scope where v8.gyp reads it ("name 'host_os' is not defined"); name the build machine's OS explicitly. Node adds the include directory of every shared library it finds through pkg-config to *all* its targets -- V8's bundled abseil too, which then picked up the abseil component's headers (same lts_ namespace) instead of its own: "redefinition of absl::...::TrailingZeros"; googletest's the same. pkg-config therefore gets a sysroot whose include directory is the staging one without the headers of what Node and V8 bundle and do not take shared.
- Node's host tools (node_js2c and friends) are linked with the same shared libraries as node itself -- for the build host, where most of them are not installed ("cannot find -lada", -lcares, -lnghttp2, -lsimdjson, -lsimdutf, -lsqlite3, -luv) -- or not in the version the staged headers are (ICU 76: "undefined reference to icu_76::UnicodeSet..."). Build those for the host from the same sources and versions as their own components (their recipes say which), into one prefix, and give it to the host link (LDFLAGS_host; rpath, so they run).
- The target compilers are the SDP's GCC driver (<triple>-gcc/g++) rather than qcc/q++: gyp's Make generator writes dependency files with -MMD -MF, which qcc misreads ("Can't specify ... -o and have multiple files"). It links the same runtime (libc++) as q++ -V..._cxx. Staged headers come in with -idirafter, after the include paths of the sources Node bundles: with -I, the abseil component's headers and the copy in deps/v8/third_party (the same lts_ namespace) were both included -- "redefinition of ... absl::lts_20240722::container_internal::TrailingZeros". gyp's Make generator, not --ninja: in a cross build the inspector headers target exists for the host and the target toolset, both writing the same stamp, which ninja rejects ("multiple rules generate .../js_protocol.stamp"). LDFLAGS_host is read when make runs (out/Makefile: LDFLAGS.host ?= ...). DT_RPATH (--disable-new-dtags), not DT_RUNPATH: only the former also covers the libraries those libraries load (libicuuc -> libicudata.so.76). V8's code generators are built first: their "run_" actions exist for the host and the target toolset with the same outputs, Make keeps the target copy, and under -j it ran before the host binary was linked (Error 127, "gen-regexp-special-case: No such file").

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH nodejs`.

### 5. What it installs

109 files under `/usr/local`, 1 programs, 108 headers.

```
/usr/local/bin/node
```

The complete list is `stage/<arch>/.manifest/nodejs` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa nodejs
ls dists/8.0/*/ | grep '^nodejs-'
```

This writes `nodejs-<ver>-r0.tar.gz` (run time) and `nodejs-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add nodejs
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/nodejs-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/nodejs-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | nodejs 24.14.1-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **24.14.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add nodejs=<version>`), sources in `ported/nodejs/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates nodejs`; port one with `scripts/port-update nodejs`.

<details><summary>recipe.sh</summary>

```bash
VERSION=24.14.1; GIT=https://github.com/nodejs/node.git; REF=v24.14.1
# Node.js 24 with qnx-ports' QNX support. patches/0001: the qnx-ports/node
# fork's QNX commit (libuv, V8 and the build files learn QNX); 0002-0006:
# QNX's aports changes the QNX Everywhere image builds with. The shared
# libraries of this tree are used instead of the bundled copies, as aports does.
#
# Cross build: V8's build runs generator programs (mksnapshot, torque, ...) on
# the build host, so configure gets the host compiler for those (CC_host) and
# the QNX compiler for the target.
DEPS="libuv ada brotli zlib openssl c-ares nghttp2 simdjson simdutf sqlite zstd icu"
BUILDSYS=custom
# V8's gyp files test host_os for the host toolset (mksnapshot, torque run on
# the build machine). toolchain.gypi gives it a default of OS, but in a cross
# build that default is not in scope where v8.gyp reads it ("name 'host_os' is
# not defined"); name the build machine's OS explicitly.
# Node adds the include directory of every shared library it finds through
# pkg-config to *all* its targets -- V8's bundled abseil too, which then
# picked up the abseil component's headers (same lts_ namespace) instead of
# its own: "redefinition of absl::...::TrailingZeros"; googletest's the same.
# pkg-config therefore gets a sysroot whose include directory is the staging
# one without the headers of what Node and V8 bundle and do not take shared.
NODE_BUNDLED_HEADERS="absl gtest gmock fp16 fp16.h hwy json re2 fast_float cpu_features"
_node_sysroot() {
  local r="$B/node-sysroot" e
  rm -rf "$r" && mkdir -p "$r$PREFIX/include" && ln -s "$STAGE$PREFIX/lib" "$r$PREFIX/lib" || return 1
  for e in "$STAGE$PREFIX/include"/*; do
    case " $NODE_BUNDLED_HEADERS " in *" $(basename "$e") "*) continue ;; esac
    ln -s "$e" "$r$PREFIX/include/"
  done
  echo "$r"
}
# Node's host tools (node_js2c and friends) are linked with the same shared
# libraries as node itself -- for the build host, where most of them are not
# installed ("cannot find -lada", -lcares, -lnghttp2, -lsimdjson, -lsimdutf,
# -lsqlite3, -luv) -- or not in the version the staged headers are (ICU 76:
# "undefined reference to icu_76::UnicodeSet..."). Build those for the host from the same sources and
# versions as their own components (their recipes say which), into one
# prefix, and give it to the host link (LDFLAGS_host; rpath, so they run).
NODE_HOST_LIBS="ada c-ares nghttp2 simdjson simdutf sqlite libuv icu"
_host() { env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u CPPFLAGS -u LDFLAGS -u LIBS -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH "$@"; }
_node_hostlibs() {
  local H="$1" n
  [ -f "$H/.done" ] && return 0
  mkdir -p "$H/src" || return 1
  for n in $NODE_HOST_LIBS; do
    echo "-- host library for node's build tools: $n"
    ( GIT= REF= COMMIT= TARBALL= SHA256= CFLAGS_APPEND=; CONF_ARGS_EXTRA=(); source "$WS/ports/$n/recipe.sh"
      d="$H/src/$n"; rm -rf "$d"
      if [ -n "$GIT" ]; then git clone -q --depth 1 --branch "$REF" "$GIT" "$d"
      else t="$WS/dist/$(basename "$TARBALL")"
           { [ -f "$t" ] || curl -fsSL -o "$t" "$TARBALL"; } && echo "$SHA256  $t" | sha256sum -c --status && \
           mkdir -p "$d" && tar xf "$t" -C "$d" --strip-components=1
      fi || exit 1
      if [ -f "$d/CMakeLists.txt" ]; then
        _host cmake -S "$d" -B "$d/_b" -G Ninja --no-warn-unused-cli -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_INSTALL_PREFIX="$H" -DCMAKE_INSTALL_LIBDIR=lib -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF \
          -DENABLE_LIB_ONLY=ON -DADA_TESTING=OFF -DADA_TOOLS=OFF -DADA_BENCHMARKS=OFF -DSIMDJSON_DEVELOPER_MODE=OFF \
          -DSIMDUTF_TESTS=OFF -DSIMDUTF_TOOLS=OFF -DLIBUV_BUILD_TESTS=OFF -DCARES_BUILD_TOOLS=OFF && \
        _host ninja -C "$d/_b" install
      else
        local c="$d"; [ -x "$d/icu4c/source/configure" ] && c="$d/icu4c/source"   # ICU keeps it there
        # The component's own configure options and flags (sqlite: the session
        # extension node:sqlite needs), then the host prefix.
        ( cd "$c" && _host env CFLAGS="-O2 $CFLAGS_APPEND" ./configure "${CONF_ARGS_EXTRA[@]}" \
            --prefix="$H" --libdir="$H/lib" --disable-static && _host make -j"$(nproc)" install )
      fi ) || return 1
  done
  touch "$H/.done"
}
do_configure() {
  local R; R=$(_node_sysroot) || return 1
  local HL="$BUILD_DIR/host/node-$VERSION-libs"
  _node_hostlibs "$HL" || return 1
  # Node builds in the source tree (out/); the other architecture's objects
  # must not be reused.
  rm -rf "$S/out" && cd "$S" && PKG_CONFIG_SYSROOT_DIR="$R" GYP_DEFINES="host_os=linux" CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc" CXX="$QNX_HOST/usr/bin/$QNX_TRIPLE-g++" CC_host=gcc CXX_host=g++ \
  CFLAGS="-D_QNX_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DOPENSSL_NO_ENGINE -idirafter $STAGE$PREFIX/include" \
  CXXFLAGS="-D_QNX_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DOPENSSL_NO_ENGINE -idirafter $STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib" PKG_CONFIG_LIBDIR="$PKG_CONFIG_LIBDIR" \
  python3 configure.py --prefix="$PREFIX" --dest-os=qnx --dest-cpu="$([ "$QNX_CPU" = aarch64 ] && echo arm64 || echo x64)" \
    --cross-compiling --use-prefix-to-find-headers --shared-libuv --shared-ada --shared-brotli --shared-zlib \
    --shared-openssl --shared-cares --shared-nghttp2 --shared-simdjson --shared-simdutf --shared-sqlite --shared-zstd \
    --openssl-use-def-ca-store --with-intl=system-icu --without-corepack --without-npm \
    --shared-libuv-includes="$R$PREFIX/include" --shared-libuv-libpath="$STAGE$PREFIX/lib"
}
# The target compilers are the SDP's GCC driver (<triple>-gcc/g++) rather than
# qcc/q++: gyp's Make generator writes dependency files with -MMD -MF, which qcc
# misreads ("Can't specify ... -o and have multiple files"). It links the same
# runtime (libc++) as q++ -V..._cxx.
# Staged headers come in with -idirafter, after the include paths of the
# sources Node bundles: with -I, the abseil component's headers and the copy in
# deps/v8/third_party (the same lts_ namespace) were both included --
# "redefinition of ... absl::lts_20240722::container_internal::TrailingZeros".
# gyp's Make generator, not --ninja: in a cross build the inspector headers
# target exists for the host and the target toolset, both writing the same
# stamp, which ninja rejects ("multiple rules generate .../js_protocol.stamp").
# LDFLAGS_host is read when make runs (out/Makefile: LDFLAGS.host ?= ...).
# DT_RPATH (--disable-new-dtags), not DT_RUNPATH: only the former also covers
# the libraries those libraries load (libicuuc -> libicudata.so.76).
# V8's code generators are built first: their "run_" actions exist for the
# host and the target toolset with the same outputs, Make keeps the target
# copy, and under -j it ran before the host binary was linked (Error 127,
# "gen-regexp-special-case: No such file").
NODE_HOST_TOOLS="gen-regexp-special-case torque bytecode_builtins_list_generator"
do_build() { local HL="$BUILD_DIR/host/node-$VERSION-libs"
  LDFLAGS_host="-L$HL/lib -Wl,-rpath,$HL/lib -Wl,--disable-new-dtags" make -C "$S/out" BUILDTYPE=Release -j"$(nproc)" $NODE_HOST_TOOLS && \
  LDFLAGS_host="-L$HL/lib -Wl,-rpath,$HL/lib -Wl,--disable-new-dtags" make -C "$S/out" BUILDTYPE=Release -j"$(nproc)"; }
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$S/out/Release/node" "$STAGE$PREFIX/bin/" && \
  cd "$S" && python3 tools/install.py install --dest-dir "$STAGE" --prefix "$PREFIX" --headers-only; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
