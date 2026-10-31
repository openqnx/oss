# sysbench 1.0.20-git-20250309

> Scriptable multi-threaded benchmark tool for CPU, memory, I/O and threads

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/akopytov/sysbench.git |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | [`luajit`](../luajit/) |
| Run-time dependencies | [`luajit`](../luajit/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_sysbench`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `sysbench`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`sysbench`](https://openqnx.github.io/oss/package/x86_64/sysbench.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/sysbench.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add sysbench` (dependencies come along); one version: `apk add sysbench=<version>` | `apk del sysbench` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install sysbench` (dependencies first) | `oss-get remove sysbench` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-sysbench && /usr/local/bin/qnx-port-tests/test_sysbench    # its on-target test
apk del sysbench qnx-test-sysbench
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/sysbench.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/sysbench.list
rm var/lib/manual/sysbench.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild sysbench` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild luajit
```

### 2. Get the upstream source

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/akopytov/sysbench.git src/sysbench
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/sysbench < ports/sysbench/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/sysbench < ports/sysbench/patches/0002-qnx-config-sh-destdir.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Adding QNX support to sysbench

> Imported from qnx-ports/sysbench (3ceba0b1e115...qnx-1.0.20); original authorship kept.
>
>
> * Adding QNX support
>
> * Fixing some conditional and whitespacing issues.
> ---
>  Makefile.am                                |  5 ++++
>  configure.ac                               |  5 ++++
>  src/lua/internal/Makefile.am               |  2 +-
>  src/sb_timer.c                             | 21 +++++++++++++
>  src/sb_timer.h                             | 11 +++++++
>  tests/Makefile.am                          |  4 ++-
>  tests/t/api_rand.t                         |  2 +-
>  third_party/concurrency_kit/ck/configure   | 34 +++++++++++++++++-----
>  third_party/luajit/Makefile.am             | 19 +++++++++---
>  third_party/luajit/luajit/src/Makefile     | 29 ++++++++++++++++--
>  third_party/luajit/luajit/src/lj_profile.c |  2 ++
>  11 files changed, 117 insertions(+), 17 deletions(-)

Touches: `Makefile.am`, `configure.ac`, `src/lua/internal/Makefile.am`, `src/sb_timer.c`, `src/sb_timer.h`, `tests/Makefile.am` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-config-sh-destdir.patch`](patches/0002-qnx-config-sh-destdir.patch)** — Install the QNX test config.sh under INSTALL_TO_DIR (honour DESTDIR).

> The QNX change installs include/config.sh straight into $(testdir), the
> final location, ignoring DESTDIR -- a staged install then tries to write to
> /usr/local on the build machine. INSTALL_TO_DIR is where the rest of the test
> files go.

Touches: `tests/Makefile.am` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/sysbench; B=build/$QNX_ARCH/sysbench; mkdir -p $B
```

```bash
do_build() { QNXENV=true make -C "$B" -j"$(nproc)"; }
```

```bash
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- sysbench builds its bundled ConcurrencyKit with ConcurrencyKit's own configure, which recognises compilers by name and rejects "qcc". The GCC driver qcc runs is named like a GCC and works directly.
- The QNX patch makes ConcurrencyKit's configure (run during "make") skip its run-time compiler probe when QNXENV=true is set -- a cross build cannot run what it compiles.
- The patches change tests/Makefile.am; regenerate the build files.
- LuaJIT's FFI (ffi.C.*) looks up sysbench's own functions in the executable, so they must be exported. sb_luajit.m4 adds -rdynamic only on Linux and FreeBSD; without it every Lua script died "Symbol not found" (hazard 122).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH sysbench`.

### 5. What it installs

69 files under `/usr/local`, 1 programs.

```
/usr/local/bin/sysbench
```

The complete list is `stage/<arch>/.manifest/sysbench` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa sysbench
ls dists/8.0/*/ | grep '^sysbench-'
```

This writes `sysbench-<ver>-r0.tar.gz` (run time) and `sysbench-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add sysbench
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/sysbench-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/sysbench-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_sysbench`](../../tests/src/) links sysbench the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-sysbench`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-sysbench
/usr/local/bin/qnx-port-tests/test_sysbench
```

```
== sysbench ==
  ok   ...
PASS sysbench
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_sysbench` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_sysbench`](../../tests/src/) -- a program linked against sysbench the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | sysbench 1.0.20_git20250309-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/sysbench-1.0.20_git20250309-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | sysbench 1.0.20_git20250309-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/sysbench-1.0.20_git20250309-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | sysbench 1.0.20_git20250309-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/sysbench-1.0.20_git20250309-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | sysbench 1.0.20_git20250309-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/sysbench-1.0.20_git20250309-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | sysbench 1.0.20_git20250309-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/sysbench-1.0.20_git20250309-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.0.20-git-20250309**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add sysbench=<version>`), sources in `ported/sysbench/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates sysbench`; port one with `scripts/port-update sysbench`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.0.20-git-20250309; GIT=https://github.com/akopytov/sysbench.git; COMMIT=3ceba0b1e115f8c50d1d045a4574d8ed643bd497
DEPS="luajit"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --without-mysql --without-pgsql --with-system-luajit )
# sysbench builds its bundled ConcurrencyKit with ConcurrencyKit's own
# configure, which recognises compilers by name and rejects "qcc". The GCC
# driver qcc runs is named like a GCC and works directly.
CONF_ENV=( QNXENV=true CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc" LUAJIT_CFLAGS="-I$STAGE$PREFIX/include/luajit-2.1" LUAJIT_LIBS="-L$STAGE$PREFIX/lib -lluajit-5.1" )
# The QNX patch makes ConcurrencyKit's configure (run during "make") skip its
# run-time compiler probe when QNXENV=true is set -- a cross build cannot run
# what it compiles.
do_build() { QNXENV=true make -C "$B" -j"$(nproc)"; }
# The patches change tests/Makefile.am; regenerate the build files.
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
# LuaJIT's FFI (ffi.C.*) looks up sysbench's own functions in the executable,
# so they must be exported. sb_luajit.m4 adds -rdynamic only on Linux and
# FreeBSD; without it every Lua script died "Symbol not found" (hazard 122).
LDFLAGS_APPEND="-Wl,--export-dynamic"
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
