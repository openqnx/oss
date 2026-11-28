# qnx-desktop-developer 1.0

> Developer tools on the target: compiler, CMake, Python, Git, GDB, Node.js

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`native-toolchain`](../native-toolchain/) [`cmake`](../cmake/) [`ninja`](../ninja/) [`meson`](../meson/) [`python3`](../python3/) [`python3-pip`](../python3-pip/) [`python3-setuptools`](../python3-setuptools/) [`python3-wheel`](../python3-wheel/) [`python3-virtualenv`](../python3-virtualenv/) [`numpy`](../numpy/) [`pygame`](../pygame/) [`git`](../git/) [`gdb`](../gdb/) [`nodejs`](../nodejs/) [`npm`](../npm/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_qnx_desktop`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `qnx-desktop-developer` and `qnx-desktop-developer-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild qnx-desktop-developer` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (79 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild native-toolchain cmake ninja meson python3 python3-pip python3-setuptools python3-wheel python3-virtualenv numpy pygame git gdb nodejs npm
```

<details><summary>Full build chain (79)</summary>

[`ada`](../ada/) [`aom`](../aom/) [`binutils`](../binutils/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`cmake`](../cmake/) [`curl`](../curl/) [`dav1d`](../dav1d/) [`expat`](../expat/) [`freetype`](../freetype/) [`gdb`](../gdb/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`git`](../git/) [`glib`](../glib/) [`gmp`](../gmp/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`highway`](../highway/) [`icu`](../icu/) [`lcms2`](../lcms2/) [`libavif`](../libavif/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libjxl`](../libjxl/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libuv`](../libuv/) [`libwebp`](../libwebp/) [`libxcrypt`](../libxcrypt/) [`libxml2`](../libxml2/) [`libyuv`](../libyuv/) [`llvm`](../llvm/) [`make`](../make/) [`meson`](../meson/) [`mpfr`](../mpfr/) [`msgpack-c`](../msgpack-c/) [`native-toolchain`](../native-toolchain/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`ninja`](../ninja/) [`nodejs`](../nodejs/) [`npm`](../npm/) [`numpy`](../numpy/) [`openblas`](../openblas/) [`openssl`](../openssl/) [`opus`](../opus/) [`opusfile`](../opusfile/) [`pcre2`](../pcre2/) [`perl`](../perl/) [`perl-error`](../perl-error/) [`pkgconf`](../pkgconf/) [`pygame`](../pygame/) [`python3`](../python3/) [`python3-distlib`](../python3-distlib/) [`python3-filelock`](../python3-filelock/) [`python3-packaging`](../python3-packaging/) [`python3-pip`](../python3-pip/) [`python3-platformdirs`](../python3-platformdirs/) [`python3-setuptools`](../python3-setuptools/) [`python3-virtualenv`](../python3-virtualenv/) [`python3-wheel`](../python3-wheel/) [`readline`](../readline/) [`sdl2`](../sdl2/) [`sdl2-image`](../sdl2-image/) [`sdl2-mixer`](../sdl2-mixer/) [`sdl2-ttf`](../sdl2-ttf/) [`simdjson`](../simdjson/) [`simdutf`](../simdutf/) [`sqlite`](../sqlite/) [`xxhash`](../xxhash/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

```bash
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop-developer/files/." "$SRCROOT/"; }
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/qnx-desktop-developer; B=build/$QNX_ARCH/qnx-desktop-developer; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/developer.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/developer.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/developer.list"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- qnx-desktop-developer: building and debugging on the target -- Raspberry Pi OS' rpd-developer plus what its base image already carries. cc/c++ (clang set up for QNX: native-toolchain brings llvm, binutils, make and pkg-config), CMake, Ninja and Meson, Python 3 with pip, setuptools, wheel, virtualenv, NumPy and pygame, Git, GDB, Node.js with npm. The compiler needs QNX's headers and link libraries, which no package carries: scripts/target-sysroot copies them from an SDP (see native-toolchain). This component holds no program of its own: it is a meta package. Installing it installs the components below (DEPS builds them first, RUNTIME_DEPS is what the package depends on; keep the two lines the same). To add when ported (not in ports/ yet -- move a name into DEPS and RUNTIME_DEPS once ports/<name> exists and its package is verified): a GPIO module for Python (gpiozero / RPi.GPIO counterpart on QNX's gpio-rp1) libcamera tools and a Python camera module (libcamera is ported for aarch64le only) python3-pyqt, python3-opengl
- The package must hold a file to exist (qnxpkg writes no package for an empty file list): the member list, share/qnx-desktop/developer.list -- the header from files/ and one member per line, written from RUNTIME_DEPS so that the list on the target is what the package depends on.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH qnx-desktop-developer`.

### 5. What it installs

1 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/qnx-desktop-developer` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa qnx-desktop-developer
ls dists/8.0/*/ | grep '^qnx-desktop-developer-'
```

This writes `qnx-desktop-developer-<ver>-r0.tar.gz` (run time) and `qnx-desktop-developer-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add qnx-desktop-developer
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/qnx-desktop-developer-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/qnx-desktop-developer-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_qnx_desktop`](../../tests/src/) links qnx-desktop-developer the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-qnx-desktop`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-qnx-desktop
/usr/local/bin/qnx-port-tests/test_qnx_desktop
```

```
== qnx-desktop-developer ==
  ok   ...
PASS qnx-desktop-developer
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_qnx_desktop` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_qnx_desktop`](../../tests/src/) -- a program linked against qnx-desktop-developer the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> qnx-desktop-developer`.

## Versions

This recipe builds **1.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add qnx-desktop-developer=<version>`), sources in `ported/qnx-desktop-developer/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates qnx-desktop-developer`; port one with `scripts/port-update qnx-desktop-developer`.

<details><summary>recipe.sh</summary>

```bash
# qnx-desktop-developer: building and debugging on the target -- Raspberry Pi OS'
# rpd-developer plus what its base image already carries. cc/c++ (clang set up
# for QNX: native-toolchain brings llvm, binutils, make and pkg-config), CMake,
# Ninja and Meson, Python 3 with pip, setuptools, wheel, virtualenv, NumPy and
# pygame, Git, GDB, Node.js with npm.
# The compiler needs QNX's headers and link libraries, which no package
# carries: scripts/target-sysroot copies them from an SDP (see native-toolchain).
# This component holds no program of its own: it is a meta package. Installing
# it installs the components below (DEPS builds them first, RUNTIME_DEPS is
# what the package depends on; keep the two lines the same).
#
# To add when ported (not in ports/ yet -- move a name into DEPS and
# RUNTIME_DEPS once ports/<name> exists and its package is verified):
#   a GPIO module for Python (gpiozero / RPi.GPIO counterpart on QNX's gpio-rp1)
#   libcamera tools and a Python camera module (libcamera is ported for aarch64le only)
#   python3-pyqt, python3-opengl
VERSION=1.0
DEPS="native-toolchain cmake ninja meson python3 python3-pip python3-setuptools python3-wheel python3-virtualenv numpy pygame git gdb nodejs npm"
RUNTIME_DEPS="native-toolchain cmake ninja meson python3 python3-pip python3-setuptools python3-wheel python3-virtualenv numpy pygame git gdb nodejs npm"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop-developer/files/." "$SRCROOT/"; }
do_build() { :; }
# The package must hold a file to exist (qnxpkg writes no package for an empty
# file list): the member list, share/qnx-desktop/developer.list -- the header
# from files/ and one member per line, written from RUNTIME_DEPS so that the
# list on the target is what the package depends on.
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/developer.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/developer.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/developer.list"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
