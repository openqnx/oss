# boost 1.86.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.gz |
| SHA-256 | `2575e74ffc3ef1cd0babac2c1ee8bdb5782a0ee672b1912da40e5b4b591ca01f` |
| Build system | custom (see recipe) |
| QNX patches | 4 |
| Build dependencies | [`zlib`](../zlib/) [`bzip2`](../bzip2/) [`xz`](../xz/) |
| Run-time dependencies | [`bzip2`](../bzip2/) [`xz`](../xz/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_boost`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install boost
oss-get install boost-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild boost` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (7 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib bzip2 xz
```

<details><summary>Full build chain (7)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.gz
mkdir -p dist src/boost
curl -fL -o dist/$(basename $URL) $URL
echo "2575e74ffc3ef1cd0babac2c1ee8bdb5782a0ee672b1912da40e5b4b591ca01f  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/boost --strip-components=1
```

### 3. Apply the QNX patches

4 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/boost < ports/boost/patches/0001-qnxnto-no-librt.patch
patch -p1 -d src/boost < ports/boost/patches/0002-asio-qnx-no-sa-restart.patch
patch -p1 -d src/boost < ports/boost/patches/0003-asio-experimental-string-view-has-include.patch
patch -p1 -d src/boost < ports/boost/patches/0004-process-qnx-procfs.patch
```

**[`0001-qnxnto-no-librt.patch`](patches/0001-qnxnto-no-librt.patch)** — b2: do not link -lrt for target-os=qnxnto.

> QNX 8.0 has no librt (clock_gettime, shm_open and the timers are in libc).
> b2 already exempts target-os=qnx from the generic "-pthread -lrt" rule but not
> qnxnto, which Boost.Log's Jamfile requires.

Touches: `tools/build/src/tools/gcc.jam` · Upstream status: QNX-specific

**[`0002-asio-qnx-no-sa-restart.patch`](patches/0002-asio-qnx-no-sa-restart.patch)** — asio: define SA_RESTART as 0 on QNX.

> QNX 8.0 does not implement SA_RESTART (its <signal.h> leaves it out), so
> signal_set_base::flags does not compile. A zero bit means "do not request
> restart", which is the behaviour QNX has.

Touches: `boost/asio/detail/socket_types.hpp` · Upstream status: QNX-specific

**[`0003-asio-experimental-string-view-has-include.patch`](patches/0003-asio-experimental-string-view-has-include.patch)** — asio: check for <experimental/string_view> before using it with GCC.

> The GCC branch of asio's config assumes libstdc++ and enables
> BOOST_ASIO_HAS_STD_EXPERIMENTAL_STRING_VIEW unconditionally. QNX pairs GCC with
> libc++, which has no <experimental/string_view>; test with __has_include as
> the clang branch already does.

Touches: `boost/asio/detail/config.hpp` · Upstream status: Pending

**[`0004-process-qnx-procfs.patch`](patches/0004-process-qnx-procfs.patch)** — Boost.Process: process enumeration on QNX.

> QNX's /proc has no Linux-style per-process "stat" file; process data is read
> with devctl(DCMD_PROC_INFO) on /proc/<pid>/as. Implement all_pids(),
> parent_pid() and the related functions that way so Boost.Process v2 builds
> and works on QNX.

Touches: `libs/process/src/pid.cpp`, `libs/process/src/ext/exe.cpp`, `libs/process/src/ext/cmd.cpp`, `libs/process/src/ext/cwd.cpp`, `libs/process/src/ext/env.cpp` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/boost; B=build/$QNX_ARCH/boost; mkdir -p $B
```

```bash
do_configure() {
  cd "$S"
  [ -x ./b2 ] || ./bootstrap.sh --with-toolset=gcc --without-libraries=python
  mkdir -p "$B"
  cat > "$B/user-config.jam" <<JAM
using gcc : qnx : $QNX_HOST/usr/bin/$QNX_TRIPLE-g++ :
  <cxxflags>-D_QNX_SOURCE
  <cxxflags>-Wno-ignored-attributes
  <linkflags>-lsocket
  <linkflags>-lm
  <archiver>$QNX_HOST/usr/bin/nto$QNX_CPU-ar
  <ranlib>$QNX_HOST/usr/bin/nto$QNX_CPU-ranlib
  ;
JAM
}
```

```bash
do_build()   { cd "$S"; _b2_args; ./b2 "${B2_ARGS[@]}"; }
```

```bash
do_install() { cd "$S"; _b2_args; ./b2 "${B2_ARGS[@]}" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): xz
- Build every compiled Boost library except python, which would need a target Python interpreter and headers to link against. Leaving BOOST_LIBS empty makes b2 build its default set (everything), rather than the --with-<lib> subset the automotive middleware alone needs -- a consumer reaching for boost_json, boost_url, boost_context or boost_locale should find them staged.
- NOTE: b2 5.2.1 (bundled with Boost 1.86) segfaults during configuration of its own "qcc" toolset, with or without a user-config. The QNX cross-GCC driver behind qcc works fine when declared as a *named gcc toolset*, so use that.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH boost`.

### 5. What it installs

16021 files under `/usr/local`: 78 shared-library entries, 15810 headers.

```
/usr/local/lib/libboost_atomic.so
/usr/local/lib/libboost_atomic.so.1.86.0
/usr/local/lib/libboost_charconv.so
/usr/local/lib/libboost_charconv.so.1.86.0
/usr/local/lib/libboost_chrono.so
/usr/local/lib/libboost_chrono.so.1.86.0
/usr/local/lib/libboost_container.so
/usr/local/lib/libboost_container.so.1.86.0
/usr/local/lib/libboost_context.so
/usr/local/lib/libboost_context.so.1.86.0
/usr/local/lib/libboost_contract.so
/usr/local/lib/libboost_contract.so.1.86.0
/usr/local/lib/libboost_coroutine.so
/usr/local/lib/libboost_coroutine.so.1.86.0
/usr/local/lib/libboost_date_time.so
/usr/local/lib/libboost_date_time.so.1.86.0
...
```

The complete list is `stage/<arch>/.manifest/boost` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH boost
ls repo/8.0/*/ | grep '^boost-'
```

This writes `boost-<ver>-r0.tar.gz` (run time) and `boost-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install boost
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/boost-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/boost-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_boost`](../../tests/src/) links boost the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_boost root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_boost'
```

```
== boost ==
  ok   ...
PASS boost
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.86.0
# Also linked at run time (found by scripts/gen-dependency-index): xz
DEPS="zlib bzip2 xz"
TARBALL=https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.gz
SHA256=2575e74ffc3ef1cd0babac2c1ee8bdb5782a0ee672b1912da40e5b4b591ca01f
BUILDSYS=custom
# Build every compiled Boost library except python, which would need a target
# Python interpreter and headers to link against. Leaving BOOST_LIBS empty makes
# b2 build its default set (everything), rather than the --with-<lib> subset the
# automotive middleware alone needs -- a consumer reaching for boost_json,
# boost_url, boost_context or boost_locale should find them staged.
BOOST_LIBS=""

# NOTE: b2 5.2.1 (bundled with Boost 1.86) segfaults during configuration of its
# own "qcc" toolset, with or without a user-config. The QNX cross-GCC driver
# behind qcc works fine when declared as a *named gcc toolset*, so use that.
_b2_args() {
  B2_ARGS=( -q -j"$(nproc)"
    --build-dir="$B" --user-config="$B/user-config.jam"
    toolset=gcc-qnx target-os=qnxnto
    # Boost.Context picks its assembly from <abi>, and b2 derives the default
    # from the *build host's* platform -- so a cross build to aarch64 asks for
    # sysv/arm64 assembly, which does not exist, and the whole build dies with
    # "No best alternative for libs/context/build/asm_sources".
    "abi=$( [ "$QNX_ARCH" = aarch64le ] && echo aapcs || echo sysv )"
    variant=release link=shared runtime-link=shared threading=multi
    cxxstd=17 define=BOOST_SYSTEM_NO_DEPRECATED
    # Boost.Iostreams probes for zlib/bzip2 in the compiler's default paths, so
    # without these it linked the SDP's prebuilt zlib -- whose soname is
    # libz.so.2 against upstream's libz.so.1 -- and libboost_iostreams then
    # loaded the SDP copy on the target. Same for bzip2.
    "-sZLIB_INCLUDE=$STAGE$PREFIX/include"  "-sZLIB_LIBPATH=$STAGE$PREFIX/lib"
    "-sBZIP2_INCLUDE=$STAGE$PREFIX/include" "-sBZIP2_LIBPATH=$STAGE$PREFIX/lib"
    --prefix="$STAGE$PREFIX"
    --includedir="$STAGE$PREFIX/include"
    --libdir="$STAGE$PREFIX/lib"
    --without-python )
  for l in $BOOST_LIBS; do B2_ARGS+=( "--with-$l" ); done
}

do_configure() {
  cd "$S"
  [ -x ./b2 ] || ./bootstrap.sh --with-toolset=gcc --without-libraries=python
  mkdir -p "$B"
  cat > "$B/user-config.jam" <<JAM
using gcc : qnx : $QNX_HOST/usr/bin/$QNX_TRIPLE-g++ :
  <cxxflags>-D_QNX_SOURCE
  <cxxflags>-Wno-ignored-attributes
  <linkflags>-lsocket
  <linkflags>-lm
  <archiver>$QNX_HOST/usr/bin/nto$QNX_CPU-ar
  <ranlib>$QNX_HOST/usr/bin/nto$QNX_CPU-ranlib
  ;
JAM
}

do_build()   { cd "$S"; _b2_args; ./b2 "${B2_ARGS[@]}"; }
do_install() { cd "$S"; _b2_args; ./b2 "${B2_ARGS[@]}" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
