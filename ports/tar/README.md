# tar 1.35

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/tar/tar-1.35.tar.xz |
| SHA-256 | `4d62ff37342ec7aed748535323930c7cf94acf71c3591882b26a7ea50f3edc16` |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | [`libiconv`](../libiconv/) |
| Run-time dependencies | [`libiconv`](../libiconv/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install tar
oss-get install tar-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild tar` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libiconv
```

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/tar/tar-1.35.tar.xz
mkdir -p dist src/tar
curl -fL -o dist/$(basename $URL) $URL
echo "4d62ff37342ec7aed748535323930c7cf94acf71c3591882b26a7ea50f3edc16  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/tar --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/tar < ports/tar/patches/0001-aports-dir_open.patch
patch -p1 -d src/tar < ports/tar/patches/0002-aports-fix-qnx-makedev.patch
```

**[`0001-aports-dir_open.patch`](patches/0001-aports-dir_open.patch)** — QNX change from QNX's aports (core/tar/dir_open.patch).

> Taken from qnx-ports/aports core/tar -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `src/create.c` · Upstream status: QNX-specific

**[`0002-aports-fix-qnx-makedev.patch`](patches/0002-aports-fix-qnx-makedev.patch)** — QNX change from QNX's aports (core/tar/fix-qnx-makedev.patch).

> Taken from qnx-ports/aports core/tar -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `src/list.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/tar && cd build/$QNX_ARCH/tar
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
LIBS="-liconv" \
$OLDPWD/src/tar/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-nls
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GNU tar. Version as in the QNX Everywhere 8.0.4r1 image.
- Also linked at run time (found by scripts/gen-dependency-index): libiconv
- The libiconv component's <iconv.h> renames iconv_open to libiconv_open, so link that library explicitly.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH tar`.

### 5. What it installs

6 files under `/usr/local`, 1 programs.

```
/usr/local/bin/tar
```

The complete list is `stage/<arch>/.manifest/tar` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH tar
ls repo/8.0/*/ | grep '^tar-'
```

This writes `tar-<ver>-r0.tar.gz` (run time) and `tar-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install tar
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/tar-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/tar-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# GNU tar. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.35
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="libiconv"
TARBALL=https://ftp.gnu.org/gnu/tar/tar-1.35.tar.xz
SHA256=4d62ff37342ec7aed748535323930c7cf94acf71c3591882b26a7ea50f3edc16
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls )
# The libiconv component's <iconv.h> renames iconv_open to libiconv_open, so
# link that library explicitly.
CONF_ENV=( LIBS="-liconv" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
