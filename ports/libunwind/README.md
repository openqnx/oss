# libunwind 1.8.3

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/libunwind/libunwind.git (tag `v1.8.3`) |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libunwind`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libunwind
apk add libunwind-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libunwind && /usr/local/bin/qnx-port-tests/test_libunwind    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libunwind`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libunwind` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v1.8.3 --recurse-submodules --shallow-submodules \
    https://github.com/libunwind/libunwind.git src/libunwind
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/libunwind && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/libunwind && cd build/$QNX_ARCH/libunwind
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libunwind/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --prefix="$UNW_PREFIX" \
    --disable-tests \
    --disable-documentation \
    --disable-minidebuginfo \
    --disable-zlibdebuginfo \
    --enable-shared \
    --enable-static
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libunwind: call-chain unwinding (unw_backtrace, unw_step, remote and core file unwinding). The QNX SDP ships libunwind 1.8.2 as a system library (/usr/lib/libunwind.so.8, libunwind-nto, libunwind-coredump: QNX's build); this is upstream's newer release, built from source -- its QNX support (the nto remote target) is upstream's own. It gets a prefix of its own, /usr/local/opt/libunwind, so it never takes the place of the system library: a program uses it on purpose, with -I/-L.../opt/libunwind/ {include,lib} and LD_LIBRARY_PATH or a run path. libtool numbers QNX libraries by their interface number: this one is libunwind.so.10 (the SDP's QNX build: libunwind.so.8, the same interface); libunwind-nto.so.0 and libunwind-coredump.so.0 have the SDP's names, one more reason for the prefix of their own.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libunwind`.

### 5. What it installs

24 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/libunwind` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libunwind
ls dists/8.0/*/ | grep '^libunwind-'
```

This writes `libunwind-<ver>-r0.tar.gz` (run time) and `libunwind-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libunwind
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libunwind-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libunwind-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libunwind`](../../tests/src/) links libunwind the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libunwind`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libunwind
/usr/local/bin/qnx-port-tests/test_libunwind
```

```
== libunwind ==
  ok   ...
PASS libunwind
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libunwind` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libunwind`](../../tests/src/) -- a program linked against libunwind the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - unw_getcontext
   - unw_init_local
   - unw_backtrace saw the call chain
   - the opt/libunwind library answered
   - dladdr(unw_backtrace)
   - built against 1.8.3

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libunwind 1.8.3-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libunwind 1.8.3-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.8.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libunwind=<version>`), sources in `ported/libunwind/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libunwind`; port one with `scripts/port-update libunwind`.

<details><summary>recipe.sh</summary>

```bash
# libunwind: call-chain unwinding (unw_backtrace, unw_step, remote and core
# file unwinding). The QNX SDP ships libunwind 1.8.2 as a system library
# (/usr/lib/libunwind.so.8, libunwind-nto, libunwind-coredump: QNX's build);
# this is upstream's newer release, built from source -- its QNX support
# (the nto remote target) is upstream's own. It gets a prefix of its own,
# /usr/local/opt/libunwind, so it never takes the place of the system
# library: a program uses it on purpose, with -I/-L.../opt/libunwind/
# {include,lib} and LD_LIBRARY_PATH or a run path. libtool numbers QNX
# libraries by their interface number: this one is libunwind.so.10 (the
# SDP's QNX build: libunwind.so.8, the same interface); libunwind-nto.so.0
# and libunwind-coredump.so.0 have the SDP's names, one more reason for
# the prefix of their own.
VERSION=1.8.3; GIT=https://github.com/libunwind/libunwind.git; REF=v1.8.3
DEPS=""
BUILDSYS=autotools
UNW_PREFIX="$PREFIX/opt/libunwind"
CONF_ARGS_EXTRA=( --prefix="$UNW_PREFIX" --disable-tests --disable-documentation
                  --disable-minidebuginfo --disable-zlibdebuginfo --enable-shared --enable-static )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
