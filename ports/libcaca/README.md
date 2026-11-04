# libcaca 0.99.beta20

> Graphics library that outputs text instead of pixels

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/cacalabs/libcaca.git (tag `v0.99.beta20`) |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`ncurses`](../ncurses/) [`zlib`](../zlib/) |
| Run-time dependencies | [`ncurses`](../ncurses/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libcaca`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libcaca` (programs, shared libraries, run-time data) and `libcaca-dev` (headers, pkg-config/CMake files, static libraries; needs `libcaca`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libcaca`](https://openqnx.github.io/oss/package/x86_64/libcaca.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libcaca.html)) · [`libcaca-dev`](https://openqnx.github.io/oss/package/x86_64/libcaca-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libcaca-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libcaca` (dependencies come along); one version: `apk add libcaca=<version>` | `apk del libcaca` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libcaca` (dependencies first) | `oss-get remove libcaca` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libcaca-dev` the same way, by its own name (`apk add libcaca-dev`, `oss-get install libcaca-dev`, by hand with its own file and list `libcaca-dev.list`); remove it before `libcaca`, which it needs.

```sh
# on the QNX target, as root
apk add libcaca libcaca-dev
apk add qnx-test-libcaca && /usr/local/bin/qnx-port-tests/test_libcaca    # its on-target test, from tests-8.0-<arch>
apk del libcaca libcaca-dev qnx-test-libcaca
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libcaca.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libcaca.list
rm var/lib/manual/libcaca.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libcaca` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild ncurses zlib
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.99.beta20 --recurse-submodules --shallow-submodules \
    https://github.com/cacalabs/libcaca.git src/libcaca
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libcaca < ports/libcaca/patches/0001-common-image-no-internal-alloc.patch
```

**[`0001-common-image-no-internal-alloc.patch`](patches/0001-common-image-no-internal-alloc.patch)** — Do not call libcaca's internal _caca_alloc2d() from img2txt/cacaview

> img2txt and cacaview (src/common-image.c) call _caca_alloc2d(), a function
> internal to libcaca that is not part of its API. libcaca is built with
> -fvisibility=hidden wherever the compiler supports it (QNX's GCC does), so the
> symbol is not exported from libcaca.so and the programs fail to link with
> "undefined reference to `_caca_alloc2d'". Allocate the pixel buffer in the
> program itself, with the same multiplication-overflow check.

Touches: `src/common-image.c` · Upstream status: Pending (not QNX-specific: any build with -fvisibility=hidden)

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/libcaca && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/libcaca && cd build/$QNX_ARCH/libcaca
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libcaca/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-static \
    --enable-ncurses \
    --enable-cxx \
    --disable-x11 \
    --disable-gl \
    --disable-slang \
    --disable-cocoa \
    --disable-java \
    --disable-python \
    --disable-ruby \
    --disable-csharp \
    --disable-imlib2 \
    --disable-doc \
    --disable-cppunit \
    --disable-zzuf
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libcaca: a graphics library that outputs text instead of pixels -- ASCII/ANSI art from images and canvases, in a terminal (ncurses) or exported to text, ANSI, HTML, SVG ...; with img2txt, cacaview, cacademo, figlet-like toilet fonts (cacafire, cacaclock).
- - the ncurses terminal driver and the C++ binding; no X11/OpenGL/slang drivers (no X server on QNX targets; slang is not ported), no Java/Python/Ruby/C# bindings, no Imlib2 (img2txt loads BMP natively); - no documentation (doxygen + LaTeX) and no unit/fuzz tests.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libcaca`.

### 5. What it installs

27 files under `/usr/local`: 4 shared-library entries, 8 programs, 5 headers.

```
/usr/local/lib/libcaca++.so
/usr/local/lib/libcaca++.so.0
/usr/local/lib/libcaca.so
/usr/local/lib/libcaca.so.0
/usr/local/bin/caca-config
/usr/local/bin/cacaclock
/usr/local/bin/cacademo
/usr/local/bin/cacafire
/usr/local/bin/cacaplay
/usr/local/bin/cacaserver
/usr/local/bin/cacaview
/usr/local/bin/img2txt
```

The complete list is `stage/<arch>/.manifest/libcaca` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libcaca
ls dists/8.0/*/ | grep '^libcaca-'
```

This writes `libcaca-<ver>-r0.tar.gz` (run time) and `libcaca-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libcaca
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libcaca-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libcaca-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libcaca`](../../tests/src/) links libcaca the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libcaca`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libcaca
/usr/local/bin/qnx-port-tests/test_libcaca
```

```
== libcaca ==
  ok   ...
PASS libcaca
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libcaca` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libcaca`](../../tests/src/) -- a program linked against libcaca the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - 12x3 canvas
   - text at (1,0)
   - line of '=' along row 2
   - exported as UTF-8 text
   - the text lines are exactly ' Hello QNX  ', blank, '============'
   - ANSI export: colour escapes and the text
   - HTML export
   - SVG export
   - text import: 2x2, 'd' at (1,1)
   - dither for 32-bit ARGB
   - the bright end is all glyphs, the dark end sparser

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> libcaca`.

## Versions

This recipe builds **0.99.beta20**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libcaca=<version>`), sources in `ported/libcaca/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libcaca`; port one with `scripts/port-update libcaca`.

<details><summary>recipe.sh</summary>

```bash
# libcaca: a graphics library that outputs text instead of pixels -- ASCII/ANSI
# art from images and canvases, in a terminal (ncurses) or exported to text,
# ANSI, HTML, SVG ...; with img2txt, cacaview, cacademo, figlet-like toilet
# fonts (cacafire, cacaclock).
VERSION=0.99.beta20; GIT=https://github.com/cacalabs/libcaca.git; REF=v0.99.beta20
DEPS="ncurses zlib"
BUILDSYS=autotools
# - the ncurses terminal driver and the C++ binding; no X11/OpenGL/slang
#   drivers (no X server on QNX targets; slang is not ported), no
#   Java/Python/Ruby/C# bindings, no Imlib2 (img2txt loads BMP natively);
# - no documentation (doxygen + LaTeX) and no unit/fuzz tests.
CONF_ARGS_EXTRA=( --disable-static --enable-ncurses --enable-cxx
                  --disable-x11 --disable-gl --disable-slang --disable-cocoa
                  --disable-java --disable-python --disable-ruby --disable-csharp
                  --disable-imlib2 --disable-doc --disable-cppunit --disable-zzuf )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
