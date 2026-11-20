# hdf5 1.14.6

> HDF5 library and tools for self-describing binary files of large numerical data (C, C++ and high-level APIs)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/HDFGroup/hdf5.git (tag `hdf5_1.14.6`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`zlib`](../zlib/) [`libaec`](../libaec/) |
| Run-time dependencies | [`libaec`](../libaec/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_hdf5`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `hdf5` (programs, shared libraries, run-time data) and `hdf5-dev` (headers, pkg-config/CMake files, static libraries; needs `hdf5`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`hdf5`](https://openqnx.github.io/oss/package/x86_64/hdf5.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/hdf5.html)) · [`hdf5-dev`](https://openqnx.github.io/oss/package/x86_64/hdf5-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/hdf5-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add hdf5` (dependencies come along); one version: `apk add hdf5=<version>` | `apk del hdf5` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install hdf5` (dependencies first) | `oss-get remove hdf5` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`hdf5-dev` the same way, by its own name (`apk add hdf5-dev`, `oss-get install hdf5-dev`, by hand with its own file and list `hdf5-dev.list`); remove it before `hdf5`, which it needs.

```sh
# on the QNX target, as root
apk add hdf5 hdf5-dev
apk add qnx-test-hdf5 && /usr/local/bin/qnx-port-tests/test_hdf5    # its on-target test, from tests-8.0-<arch>
apk del hdf5 hdf5-dev qnx-test-hdf5
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/hdf5.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/hdf5.list
rm var/lib/manual/hdf5.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild hdf5` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (2 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib libaec
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b hdf5_1.14.6 --recurse-submodules --shallow-submodules \
    https://github.com/HDFGroup/hdf5.git src/hdf5
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/hdf5 < ports/hdf5/patches/0001-pkgconfig-libaec-names.patch
```

**[`0001-pkgconfig-libaec-names.patch`](patches/0001-pkgconfig-libaec-names.patch)** — Name libsz and libaec in hdf5.pc instead of libname-NOTFOUND.

> hdf5.pc lists the compression libraries in Libs.private by reading each
> CMake target's OUTPUT_NAME. The targets libaec's package files export
> (libaec::sz, libaec::aec) are imported targets and have none, so the
> installed file said "-llibname-NOTFOUND -llibname-NOTFOUND" and a static
> or --static pkg-config consumer got a link line that cannot work. Fall
> back to the target's own name, which for libaec is the library name
> (libsz, libaec). Not QNX specific: any build against an installed libaec
> shows it; QNX is where this port builds with SZIP enabled.

Touches: `src/CMakeLists.txt` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/hdf5; B=build/$QNX_ARCH/hdf5; mkdir -p $B
```

```bash
do_build() {
  # The build record: target facts instead of build-machine paths (see above).
  # Both files are written by configure; H5build_settings.c is libhdf5.settings
  # as a C string.
  local f
  for f in "$B/src/libhdf5.settings" "$B/src/H5build_settings.c"; do
    [ -f "$f" ] || { echo "missing $f"; return 1; }
    # [^\\]*: up to the end of the line in the text file, up to the closing
    # \n" in the C string.
    sed -i -e "s#$QNX_TARGET#<qnx-target>#g" -e "s#$STAGE##g" -e "s#$QNX_HOST/usr/bin/##g" \
           -e "s#$B#<build>#g" -e "s#$S#<source>#g" \
           -e "s#\(Host system: \)[^\\]*#\1QNX 8.0 $QNX_CPU (cross-built)#" \
           -e "s#\(Configured on: \)[^\\]*#\1(not recorded)#" "$f" || return 1
  done
  cmake --build "$B" -j"$(nproc)"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- hdf5: the HDF5 library and tools -- a self-describing binary file format for large numerical data (groups, n-dimensional datasets, attributes, compound types, chunking and compression filters). Built here: the C library (libhdf5), the high-level library (libhdf5_hl: H5LT, H5TB, H5IM, H5DS, H5PT), the C++ API (libhdf5_cpp, libhdf5_hl_cpp) and the command line tools (h5dump, h5ls, h5diff, h5repack, h5copy, h5stat, ...). QNX specifics: - Cross build. HDF5's CMake files run a handful of small programs at configure time to learn how the target converts long double and _Float16 values (config/cmake/ConversionTests.c); they cannot run here. Their answers are given below as cache variables instead of being left to the "cross-compile default" branch, so the configuration is stated in one place and identical on every build machine. For x86_64 and aarch64 with GCC 12 they are the answers every Linux build of these two ABIs gets: no IBM POWER special cases, accurate long double <-> long long conversion. - _Float16 (H5T_NATIVE_FLOAT16): HDF5 switches it off when it cannot run its probe. The probe's first half is a link test (does libgcc have __extendhfsf2, __truncdfhf2, ...): compiled and linked by hand for both QNX targets, with and without optimisation, it links. The second half guards against a macOS 13 compiler defect that does not apply to GCC. So it is enabled; tests/src/test_hdf5.cpp checks a _Float16 conversion on the target against the IEEE 754 binary16 bit pattern. - H5_HAVE_LIBDL=0: dlopen() is in QNX's libc and there is no libdl. The workspace's toolchain setup provides an empty stub libdl.a for projects that insist on -ldl; HDF5's probe finds it and would record "dl" as a library HDF5 needs (link line, libhdf5.settings, hdf5-config.cmake). - libhdf5.settings (installed, and compiled into libhdf5 for H5check_version()'s diagnostics) records the build: compiler and archiver paths, link flags, the build directory and the build machine's kernel as "Host system". do_build rewrites those to what is true on the target before anything is compiled, so neither the file nor the library depends on where and when it was built. - Shared libraries only (the tools link them); examples and the test suite are not built. Thread-safe mode stays off, as in upstream's default: it cannot be combined with the C++ and high-level APIs. - SZIP filter through libaec (encoder and decoder), DEFLATE through zlib. - Install layout: CMake package files in lib/cmake/hdf5 and COPYING, RELEASE.txt in share/hdf5 (upstream's defaults are <prefix>/cmake and <prefix>/share itself, shared with every other component).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH hdf5`.

### 5. What it installs

155 files under `/usr/local`: 15 shared-library entries, 24 programs, 104 headers.

```
/usr/local/lib/libhdf5.so
/usr/local/lib/libhdf5.so.310
/usr/local/lib/libhdf5.so.310.5.1
/usr/local/lib/libhdf5_cpp.so
/usr/local/lib/libhdf5_cpp.so.310
/usr/local/lib/libhdf5_cpp.so.310.0.6
/usr/local/lib/libhdf5_hl.so
/usr/local/lib/libhdf5_hl.so.310
/usr/local/lib/libhdf5_hl.so.310.0.6
/usr/local/lib/libhdf5_hl_cpp.so
/usr/local/lib/libhdf5_hl_cpp.so.310
/usr/local/lib/libhdf5_hl_cpp.so.310.0.6
/usr/local/lib/libhdf5_tools.so
/usr/local/lib/libhdf5_tools.so.310
/usr/local/lib/libhdf5_tools.so.310.0.6
/usr/local/bin/h5c++
...
```

The complete list is `stage/<arch>/.manifest/hdf5` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa hdf5
ls dists/8.0/*/ | grep '^hdf5-'
```

This writes `hdf5-<ver>-r0.tar.gz` (run time) and `hdf5-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add hdf5
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/hdf5-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/hdf5-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_hdf5`](../../tests/src/) links hdf5 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-hdf5`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-hdf5
/usr/local/bin/qnx-port-tests/test_hdf5
```

```
== hdf5 ==
  ok   ...
PASS hdf5
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_hdf5` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_hdf5`](../../tests/src/) -- a program linked against hdf5 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - library version matches the headers
   - this is HDF5 1.14
   - DEFLATE filter encodes and decodes
   - SZIP filter encodes and decodes
   - H5Fcreate with the default file access (file locking: best effort)
   - nested groups created
   - DEFLATE+shuffle dataset is stored in less than half its raw size
   - SZIP dataset is stored smaller than raw
   - attributes written
   - compound dataset written
   - H5LT dataset and string attribute written
   - file closed
   - file starts with the HDF5 signature
   - H5Fis_accessible recognises it
   - file reopened read-only
   - DEFLATE dataset reads back identical
   - SZIP dataset reads back identical
   - strided hyperslab holds the expected cells
   - read as 64-bit integers, sum matches
   - dataset reports chunked layout with 2 filters
   - attributes read back: 12.5 and \"millimetre\"
   - compound records read back
   - /sensors has 3 links
   - H5Lexists tells present from absent
   - opening a missing dataset fails
   - ... and 21 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | hdf5 1.14.6-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/aarch64le/hdf5-1.14.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | hdf5 1.14.6-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/x86_64/hdf5-1.14.6-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.14.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add hdf5=<version>`), sources in `ported/hdf5/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates hdf5`; port one with `scripts/port-update hdf5`.

<details><summary>recipe.sh</summary>

```bash
# hdf5: the HDF5 library and tools -- a self-describing binary file format
# for large numerical data (groups, n-dimensional datasets, attributes,
# compound types, chunking and compression filters). Built here: the C
# library (libhdf5), the high-level library (libhdf5_hl: H5LT, H5TB, H5IM,
# H5DS, H5PT), the C++ API (libhdf5_cpp, libhdf5_hl_cpp) and the command
# line tools (h5dump, h5ls, h5diff, h5repack, h5copy, h5stat, ...).
#
# QNX specifics:
# - Cross build. HDF5's CMake files run a handful of small programs at
#   configure time to learn how the target converts long double and
#   _Float16 values (config/cmake/ConversionTests.c); they cannot run here.
#   Their answers are given below as cache variables instead of being left
#   to the "cross-compile default" branch, so the configuration is stated in
#   one place and identical on every build machine. For x86_64 and aarch64
#   with GCC 12 they are the answers every Linux build of these two ABIs
#   gets: no IBM POWER special cases, accurate long double <-> long long
#   conversion.
# - _Float16 (H5T_NATIVE_FLOAT16): HDF5 switches it off when it cannot run
#   its probe. The probe's first half is a link test (does libgcc have
#   __extendhfsf2, __truncdfhf2, ...): compiled and linked by hand for both
#   QNX targets, with and without optimisation, it links. The second half
#   guards against a macOS 13 compiler defect that does not apply to GCC.
#   So it is enabled; tests/src/test_hdf5.cpp checks a _Float16 conversion on
#   the target against the IEEE 754 binary16 bit pattern.
# - H5_HAVE_LIBDL=0: dlopen() is in QNX's libc and there is no libdl. The
#   workspace's toolchain setup provides an empty stub libdl.a for projects
#   that insist on -ldl; HDF5's probe finds it and would record "dl" as a
#   library HDF5 needs (link line, libhdf5.settings, hdf5-config.cmake).
# - libhdf5.settings (installed, and compiled into libhdf5 for
#   H5check_version()'s diagnostics) records the build: compiler and
#   archiver paths, link flags, the build directory and the build machine's
#   kernel as "Host system". do_build rewrites those to what is true on the
#   target before anything is compiled, so neither the file nor the library
#   depends on where and when it was built.
# - Shared libraries only (the tools link them); examples and the test
#   suite are not built. Thread-safe mode stays off, as in upstream's
#   default: it cannot be combined with the C++ and high-level APIs.
# - SZIP filter through libaec (encoder and decoder), DEFLATE through zlib.
# - Install layout: CMake package files in lib/cmake/hdf5 and COPYING,
#   RELEASE.txt in share/hdf5 (upstream's defaults are <prefix>/cmake and
#   <prefix>/share itself, shared with every other component).
VERSION=1.14.6; GIT=https://github.com/HDFGroup/hdf5.git; REF=hdf5_1.14.6
DEPS="zlib libaec"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=(
  -DBUILD_TESTING=OFF -DHDF5_BUILD_EXAMPLES=OFF
  -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF
  -DHDF5_BUILD_TOOLS=ON -DHDF5_BUILD_HL_LIB=ON -DHDF5_BUILD_CPP_LIB=ON
  -DHDF5_BUILD_FORTRAN=OFF -DHDF5_BUILD_JAVA=OFF
  -DHDF5_ENABLE_Z_LIB_SUPPORT=ON -DHDF5_ENABLE_SZIP_SUPPORT=ON -DHDF5_ENABLE_SZIP_ENCODING=ON
  -DHDF5_INSTALL_CMAKE_DIR=lib/cmake/hdf5 -DHDF5_INSTALL_DATA_DIR=share/hdf5
  -DH5_DEFAULT_PLUGINDIR=/usr/local/lib/hdf5/plugin
  # dlopen() is in libc (see above)
  -DH5_HAVE_LIBDL=0
  # config/cmake/ConversionTests.c, answered for x86_64 and aarch64 (see above).
  # An empty value is HDF5's own "no" (the macro tests the variable's value).
  -DH5_LDOUBLE_TO_LONG_SPECIAL= -DH5_LONG_TO_LDOUBLE_SPECIAL=
  -DH5_LDOUBLE_TO_LLONG_ACCURATE=1 -DH5_LLONG_TO_LDOUBLE_CORRECT=1
  -DH5_DISABLE_SOME_LDOUBLE_CONV=
  -DH5_FLOAT16_CONVERSION_FUNCS_LINK=1 -DH5_FLOAT16_CONVERSION_FUNCS_LINK_NO_FLAGS=1
  -DH5_LDOUBLE_TO_FLOAT16_CORRECT=1 -DH5_LDOUBLE_TO_FLOAT16_CORRECT_NO_FLAGS=1
)
do_build() {
  # The build record: target facts instead of build-machine paths (see above).
  # Both files are written by configure; H5build_settings.c is libhdf5.settings
  # as a C string.
  local f
  for f in "$B/src/libhdf5.settings" "$B/src/H5build_settings.c"; do
    [ -f "$f" ] || { echo "missing $f"; return 1; }
    # [^\\]*: up to the end of the line in the text file, up to the closing
    # \n" in the C string.
    sed -i -e "s#$QNX_TARGET#<qnx-target>#g" -e "s#$STAGE##g" -e "s#$QNX_HOST/usr/bin/##g" \
           -e "s#$B#<build>#g" -e "s#$S#<source>#g" \
           -e "s#\(Host system: \)[^\\]*#\1QNX 8.0 $QNX_CPU (cross-built)#" \
           -e "s#\(Configured on: \)[^\\]*#\1(not recorded)#" "$f" || return 1
  done
  cmake --build "$B" -j"$(nproc)"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
