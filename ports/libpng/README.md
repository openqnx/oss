# libpng 1.6.44

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/pnggroup/libpng.git (tag `v1.6.44`) |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) |
| Run-time dependencies | [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libpng`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libpng
oss-get install libpng-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libpng` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild zlib
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.6.44 --recurse-submodules --shallow-submodules \
    https://github.com/pnggroup/libpng.git src/libpng
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/libpng && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/libpng && cd build/$QNX_ARCH/libpng
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libpng/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --enable-static \
    --disable-tools
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libpng's CMake path generates pnglibconf by *running* a freshly built helper. That cannot work when cross-compiling -- the host's qemu binfmt handler picks up the QNX aarch64 binary and dies on /usr/lib/ldqnx-64.so.2. Its configure script handles cross builds properly and falls back to the prebuilt config.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libpng`.

### 5. What it installs

18 files under `/usr/local`: 3 shared-library entries, 2 programs, 6 headers.

```
/usr/local/lib/libpng.so
/usr/local/lib/libpng16.so
/usr/local/lib/libpng16.so.16
/usr/local/bin/libpng-config
/usr/local/bin/libpng16-config
```

The complete list is `stage/<arch>/.manifest/libpng` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libpng
ls repo/8.0/*/ | grep '^libpng-'
```

This writes `libpng-<ver>-r0.tar.gz` (run time) and `libpng-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libpng
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libpng-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libpng-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libpng`](../../tests/src/) links libpng the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libpng root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libpng'
```

```
== libpng ==
  ok   ...
PASS libpng
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.6.44; GIT=https://github.com/pnggroup/libpng.git; REF=v1.6.44
DEPS="zlib"
BUILDSYS=autotools
# libpng's CMake path generates pnglibconf by *running* a freshly built helper.
# That cannot work when cross-compiling -- the host's qemu binfmt handler picks
# up the QNX aarch64 binary and dies on /usr/lib/ldqnx-64.so.2. Its configure
# script handles cross builds properly and falls back to the prebuilt config.
CONF_ARGS_EXTRA=( --enable-shared --enable-static --disable-tools )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
