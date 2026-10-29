# lely-core 2.3.4

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://gitlab.com/lely_industries/lely-core.git (tag `v2.3.4`) |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_lely_core`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install lely-core
oss-get install lely-core-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild lely-core` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2.3.4 --recurse-submodules --shallow-submodules \
    https://gitlab.com/lely_industries/lely-core.git src/lely-core
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/lely-core < ports/lely-core/patches/0001-qnx-libc-shim-collisions.patch
```

**[`0001-qnx-libc-shim-collisions.patch`](patches/0001-qnx-libc-shim-collisions.patch)** — lely-core: do not redefine struct timespec/itimerspec on QNX.

> QNX defines them in <time.h> but gates its POSIX declarations on __EXT_POSIX*
> rather than _POSIX_C_SOURCE, so lely's probes miss them and its fallback
> definitions collide with the system ones.

Touches: `include/lely/libc/time.h`, `include/lely/libc/strings.h`, `include/lely/libc/unistd.h`, `src/coapp/node.cpp` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/lely-core && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/lely-core && cd build/$QNX_ARCH/lely-core
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/lely-core/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-cython \
    --disable-python \
    --disable-tools \
    --disable-doc \
    --disable-tests \
    --disable-daemon
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- CANopen stack. QNX has no SocketCAN, so the generic (non-socketcan) CAN backend is selected; the vehicle interface is bound at integration time. lely bundles a getopt replacement whose header declares optarg/optind/opterr/ optopt as tentative definitions. Since GCC 10 the default is -fno-common, so every translation unit that includes it emits its own definition and the library fails to link with "multiple definition of optarg".

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH lely-core`.

### 5. What it installs

227 files under `/usr/local`: 16 shared-library entries, 201 headers.

```
/usr/local/lib/liblely-can.so
/usr/local/lib/liblely-can.so.1
/usr/local/lib/liblely-co.so
/usr/local/lib/liblely-co.so.2
/usr/local/lib/liblely-coapp.so
/usr/local/lib/liblely-coapp.so.2
/usr/local/lib/liblely-ev.so
/usr/local/lib/liblely-ev.so.2
/usr/local/lib/liblely-io2.so
/usr/local/lib/liblely-io2.so.2
/usr/local/lib/liblely-libc.so
/usr/local/lib/liblely-libc.so.2
/usr/local/lib/liblely-tap.so
/usr/local/lib/liblely-tap.so.2
/usr/local/lib/liblely-util.so
/usr/local/lib/liblely-util.so.2
```

The complete list is `stage/<arch>/.manifest/lely-core` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH lely-core
ls repo/8.0/*/ | grep '^lely-core-'
```

This writes `lely-core-<ver>-r0.tar.gz` (run time) and `lely-core-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install lely-core
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/lely-core-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/lely-core-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_lely_core`](../../tests/src/) links lely-core the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_lely_core root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_lely_core'
```

```
== lely-core ==
  ok   ...
PASS lely-core
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.3.4; GIT=https://gitlab.com/lely_industries/lely-core.git; REF=v2.3.4
BUILDSYS=autotools
# CANopen stack. QNX has no SocketCAN, so the generic (non-socketcan) CAN
# backend is selected; the vehicle interface is bound at integration time.
# lely bundles a getopt replacement whose header declares optarg/optind/opterr/
# optopt as tentative definitions. Since GCC 10 the default is -fno-common, so
# every translation unit that includes it emits its own definition and the
# library fails to link with "multiple definition of optarg".
CFLAGS_APPEND="-fcommon"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-cython
                  --disable-python --disable-tools --disable-doc
                  --disable-tests --disable-daemon )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
