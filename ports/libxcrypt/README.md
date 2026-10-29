# libxcrypt 4.4.38

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/besser82/libxcrypt/releases/download/v4.4.38/libxcrypt-4.4.38.tar.xz |
| SHA-256 | `80304b9c306ea799327f01d9a7549bdb28317789182631f1b54f4511b4206dd6` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libxcrypt
oss-get install libxcrypt-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libxcrypt` performs exactly the same steps (2–5) in one command.

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
URL=https://github.com/besser82/libxcrypt/releases/download/v4.4.38/libxcrypt-4.4.38.tar.xz
mkdir -p dist src/libxcrypt
curl -fL -o dist/$(basename $URL) $URL
echo "80304b9c306ea799327f01d9a7549bdb28317789182631f1b54f4511b4206dd6  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libxcrypt --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libxcrypt < ports/libxcrypt/patches/0001-aports-printf-null.patch
```

**[`0001-aports-printf-null.patch`](patches/0001-aports-printf-null.patch)** — QNX change from QNX's aports (core/libxcrypt/printf-null.patch).

> Taken from qnx-ports/aports core/libxcrypt -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `libxcrypt-4.4.38-qnx/test/checksalt.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libxcrypt && cd build/$QNX_ARCH/libxcrypt
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libxcrypt/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-werror \
    --enable-hashes=strong,glibc \
    --enable-obsolete-api=no
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libxcrypt (modern crypt() hashes). Version as in the QNX Everywhere 8.0.4r1 image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libxcrypt`.

### 5. What it installs

16 files under `/usr/local`: 2 shared-library entries, 1 headers.

```
/usr/local/lib/libcrypt.so
/usr/local/lib/libcrypt.so.2
```

The complete list is `stage/<arch>/.manifest/libxcrypt` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libxcrypt
ls repo/8.0/*/ | grep '^libxcrypt-'
```

This writes `libxcrypt-<ver>-r0.tar.gz` (run time) and `libxcrypt-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libxcrypt
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libxcrypt-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libxcrypt-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# libxcrypt (modern crypt() hashes). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=4.4.38
TARBALL=https://github.com/besser82/libxcrypt/releases/download/v4.4.38/libxcrypt-4.4.38.tar.xz
SHA256=80304b9c306ea799327f01d9a7549bdb28317789182631f1b54f4511b4206dd6
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-werror --enable-hashes=strong,glibc --enable-obsolete-api=no )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
