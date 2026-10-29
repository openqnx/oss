# farmhash 1.1-git-20190513

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/google/farmhash.git |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install farmhash
oss-get install farmhash-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild farmhash` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/google/farmhash.git src/farmhash
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/farmhash < ports/farmhash/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/farmhash < ports/farmhash/patches/0002-qnx-bswap-builtins.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Import farmhash port

> Imported from qnx-ports/farmhash (0d859a811870...qnx); original authorship kept.
>
>
> ---
>  src/farmhash.cc | 219 +++++++++++++++++++++++++++++++++++++++++++-----
>  1 file changed, 199 insertions(+), 20 deletions(-)

Touches: `src/farmhash.cc`, `farmhash.patch` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-bswap-builtins.patch`](patches/0002-qnx-bswap-builtins.patch)** — Use GCC's byte-swap builtins on QNX.

> QNX has no <byteswap.h> (a glibc header); __builtin_bswap32/64 are what
> it would provide.

Touches: `src/farmhash.cc` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/farmhash && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/farmhash && cd build/$QNX_ARCH/farmhash
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/farmhash/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- farmhash.h's little-endian test misspells __BYTE_ORDER__ and falls through to <endian.h>, a glibc header. Both QNX targets are little endian; say so.
- The QNX patch touches configure.ac/Makefile.am; regenerate the build files (the tarball's were made with automake 1.14, which make then asks for).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH farmhash`.

### 5. What it installs

49 files under `/usr/local`: 2 shared-library entries, 1 headers.

```
/usr/local/lib/libfarmhash.so
/usr/local/lib/libfarmhash.so.0
```

The complete list is `stage/<arch>/.manifest/farmhash` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH farmhash
ls repo/8.0/*/ | grep '^farmhash-'
```

This writes `farmhash-<ver>-r0.tar.gz` (run time) and `farmhash-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install farmhash
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/farmhash-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/farmhash-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.1-git-20190513; GIT=https://github.com/google/farmhash.git; COMMIT=0d859a811870d10f53a594927d0d0b97573ad06d
BUILDSYS=autotools
# farmhash.h's little-endian test misspells __BYTE_ORDER__ and falls through to
# <endian.h>, a glibc header. Both QNX targets are little endian; say so.
CFLAGS_APPEND="-D__LITTLE_ENDIAN__"
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
# The QNX patch touches configure.ac/Makefile.am; regenerate the build files
# (the tarball's were made with automake 1.14, which make then asks for).
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
