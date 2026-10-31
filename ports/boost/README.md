# boost 1.86.0

> Boost peer-reviewed portable C++ source libraries

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

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `boost` (programs, shared libraries, run-time data) and `boost-dev` (headers, pkg-config/CMake files, static libraries; needs `boost`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`boost`](https://openqnx.github.io/oss/package/x86_64/boost.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/boost.html)) · [`boost-dev`](https://openqnx.github.io/oss/package/x86_64/boost-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/boost-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add boost` (dependencies come along); one version: `apk add boost=<version>` | `apk del boost` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install boost` (dependencies first) | `oss-get remove boost` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`boost-dev` the same way, by its own name (`apk add boost-dev`, `oss-get install boost-dev`, by hand with its own file and list `boost-dev.list`); remove it before `boost`, which it needs.

```sh
# on the QNX target, as root
apk add boost boost-dev
apk add qnx-test-boost && /usr/local/bin/qnx-port-tests/test_boost    # its on-target test
apk del boost boost-dev qnx-test-boost
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/boost.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/boost.list
rm var/lib/manual/boost.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa boost
ls dists/8.0/*/ | grep '^boost-'
```

This writes `boost-<ver>-r0.tar.gz` (run time) and `boost-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add boost
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/boost-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/boost-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_boost`](../../tests/src/) links boost the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-boost`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-boost
/usr/local/bin/qnx-port-tests/test_boost
```

```
== boost ==
  ok   ...
PASS boost
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_boost` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_boost`](../../tests/src/) -- a program linked against boost the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - thread + atomic
   - filesystem path parsing
   - regex capture
   - date_time clock

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | boost 1.86.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/boost-1.86.0-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | boost 1.86.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/boost-1.86.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.86.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add boost=<version>`), sources in `ported/boost/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates boost`; port one with `scripts/port-update boost`.

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
