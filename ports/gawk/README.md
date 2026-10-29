# gawk 5.3.2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/gawk/gawk-5.3.2.tar.xz |
| SHA-256 | `f8c3486509de705192138b00ef2c00bbbdd0e84c30d5c07d23fc73a9dc4cc9cc` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`readline`](../readline/) |
| Run-time dependencies | [`readline`](../readline/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install gawk
oss-get install gawk-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gawk` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild readline
```

<details><summary>Full build chain (2)</summary>

[`ncurses`](../ncurses/) [`readline`](../readline/)
</details>

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/gawk/gawk-5.3.2.tar.xz
mkdir -p dist src/gawk
curl -fL -o dist/$(basename $URL) $URL
echo "f8c3486509de705192138b00ef2c00bbbdd0e84c30d5c07d23fc73a9dc4cc9cc  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/gawk --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/gawk < ports/gawk/patches/0001-aports-fix-qnx.patch
```

**[`0001-aports-fix-qnx.patch`](patches/0001-aports-fix-qnx.patch)** — QNX change from QNX's aports (core/gawk/fix-qnx.patch).

> Taken from qnx-ports/aports core/gawk -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `gawk-5.3.2/awk.h`, `gawk-5.3.2/extension/inplace.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/gawk && cd build/$QNX_ARCH/gawk
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/gawk/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-nls \
    --without-mpfr
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GNU awk. Version as in the QNX Everywhere 8.0.4r1 image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gawk`.

### 5. What it installs

75 files under `/usr/local`, 4 programs, 1 headers.

```
/usr/local/bin/awk
/usr/local/bin/gawk
/usr/local/bin/gawk-5.3.2
/usr/local/bin/gawkbug
```

The complete list is `stage/<arch>/.manifest/gawk` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH gawk
ls repo/8.0/*/ | grep '^gawk-'
```

This writes `gawk-<ver>-r0.tar.gz` (run time) and `gawk-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install gawk
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/gawk-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/gawk-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# GNU awk. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=5.3.2
TARBALL=https://ftp.gnu.org/gnu/gawk/gawk-5.3.2.tar.xz
SHA256=f8c3486509de705192138b00ef2c00bbbdd0e84c30d5c07d23fc73a9dc4cc9cc
DEPS="readline"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --without-mpfr )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
