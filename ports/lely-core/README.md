# lely-core 2.3.4

> Lely C/C++ libraries for CANopen, CAN, event loop and I/O

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

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `lely-core` (programs, shared libraries, run-time data) and `lely-core-dev` (headers, pkg-config/CMake files, static libraries; needs `lely-core`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`lely-core`](https://openqnx.github.io/oss/package/x86_64/lely-core.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lely-core.html)) · [`lely-core-dev`](https://openqnx.github.io/oss/package/x86_64/lely-core-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lely-core-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add lely-core` (dependencies come along); one version: `apk add lely-core=<version>` | `apk del lely-core` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install lely-core` (dependencies first) | `oss-get remove lely-core` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`lely-core-dev` the same way, by its own name (`apk add lely-core-dev`, `oss-get install lely-core-dev`, by hand with its own file and list `lely-core-dev.list`); remove it before `lely-core`, which it needs.

```sh
# on the QNX target, as root
apk add lely-core lely-core-dev
apk add qnx-test-lely-core && /usr/local/bin/qnx-port-tests/test_lely_core    # its on-target test
apk del lely-core lely-core-dev qnx-test-lely-core
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/lely-core.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/lely-core.list
rm var/lib/manual/lely-core.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa lely-core
ls dists/8.0/*/ | grep '^lely-core-'
```

This writes `lely-core-<ver>-r0.tar.gz` (run time) and `lely-core-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add lely-core
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/lely-core-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/lely-core-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_lely_core`](../../tests/src/) links lely-core the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-lely-core`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-lely-core
/usr/local/bin/qnx-port-tests/test_lely_core
```

```
== lely-core ==
  ok   ...
PASS lely-core
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_lely_core` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_lely_core`](../../tests/src/) -- a program linked against lely-core the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - co_dev_create (node id 1)
   - node id readback
   - co_obj_create 0x1000
   - insert object into dictionary
   - find object by index
   - co_sub_create subindex 0
   - insert sub-object

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lely-core 2.3.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/lely-core-2.3.4-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lely-core 2.3.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/lely-core-2.3.4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.3.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add lely-core=<version>`), sources in `ported/lely-core/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates lely-core`; port one with `scripts/port-update lely-core`.

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
