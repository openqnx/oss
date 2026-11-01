# lmbench 3.0-git-20260929

> Suite of micro-benchmarks for operating system and hardware performance

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/intel/lmbench.git |
| Build system | custom (see recipe) |
| QNX patches | 5 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_lmbench`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `lmbench` (programs, shared libraries, run-time data) and `lmbench-dev` (headers, pkg-config/CMake files, static libraries; needs `lmbench`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`lmbench`](https://openqnx.github.io/oss/package/x86_64/lmbench.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lmbench.html)) · [`lmbench-dev`](https://openqnx.github.io/oss/package/x86_64/lmbench-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lmbench-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add lmbench` (dependencies come along); one version: `apk add lmbench=<version>` | `apk del lmbench` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install lmbench` (dependencies first) | `oss-get remove lmbench` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`lmbench-dev` the same way, by its own name (`apk add lmbench-dev`, `oss-get install lmbench-dev`, by hand with its own file and list `lmbench-dev.list`); remove it before `lmbench`, which it needs.

```sh
# on the QNX target, as root
apk add lmbench lmbench-dev
apk add qnx-test-lmbench && /usr/local/bin/qnx-port-tests/test_lmbench    # its on-target test, from tests-8.0-<arch>
apk del lmbench lmbench-dev qnx-test-lmbench
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/lmbench.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/lmbench.list
rm var/lib/manual/lmbench.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild lmbench` performs exactly the same steps (2–5) in one command.

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
    https://github.com/intel/lmbench.git src/lmbench
```

### 3. Apply the QNX patches

5 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/lmbench < ports/lmbench/patches/0001-no-rpc-build.patch
patch -p1 -d src/lmbench < ports/lmbench/patches/0002-udp-done-no-portmapper.patch
patch -p1 -d src/lmbench < ports/lmbench/patches/0003-no-sysv-semaphore-benchmark.patch
patch -p1 -d src/lmbench < ports/lmbench/patches/0004-pthread-libs-variable.patch
patch -p1 -d src/lmbench < ports/lmbench/patches/0005-bw-mem64d-delay-name.patch
```

**[`0001-no-rpc-build.patch`](patches/0001-no-rpc-build.patch)** — Build without Sun RPC (NO_RPC / NO_PORTMAPPER).

> QNX has no Sun RPC (<rpc/rpc.h>, portmapper). With -DNO_RPC bench.h does not
> include the RPC headers; with -DNO_PORTMAPPER the TCP helpers use the
> benchmark's program number as the port number instead of registering it with
> the portmapper, as the UDP helpers already do. lat_rpc, the one benchmark
> that is RPC itself, is dropped from the build.
>
> qnx-ports builds lmbench with the same NO_RPC idea (its tree is not public).

Touches: `src/bench.h`, `src/lib_tcp.c`, `src/Makefile` · Upstream status: QNX-specific

**[`0002-udp-done-no-portmapper.patch`](patches/0002-udp-done-no-portmapper.patch)** — Do not unregister from the portmapper in udp_done() with NO_PORTMAPPER.

> udp_server() already skips the portmapper with NO_PORTMAPPER; udp_done()
> still called pmap_unset(), which does not exist without Sun RPC (QNX).

Touches: `src/lib_udp.c` · Upstream status: QNX-specific

**[`0003-no-sysv-semaphore-benchmark.patch`](patches/0003-no-sysv-semaphore-benchmark.patch)** — Do not build lat_sem.

> QNX declares the System V semaphore calls (semget, semctl, semop) in
> <sys/sem.h> but no library provides them, so lat_sem cannot link. QNX's
> semaphores are POSIX ones, which lmbench does not measure.

Touches: `src/Makefile` · Upstream status: QNX-specific

**[`0004-pthread-libs-variable.patch`](patches/0004-pthread-libs-variable.patch)** — Make lat_mem_pingpong's -lpthread -lrt overridable.

> QNX has the threads and realtime calls in libc and no libpthread or librt to
> link, so "-lpthread -lrt" fails ("cannot find -lpthread"). Keep them as the
> default and let a build set PTHREAD_LIBS to what its system needs.

Touches: `src/Makefile` · Upstream status: QNX-specific

**[`0005-bw-mem64d-delay-name.patch`](patches/0005-bw-mem64d-delay-name.patch)** — Rename bw_mem64d's static delay() to nop_delay().

> QNX's <unistd.h> declares delay(unsigned int) (sleep for milliseconds), so
> the file-local delay(int) that spins for a number of NOPs conflicts with it:
> conflicting types for 'delay'.

Touches: `src/bw_mem64d.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/lmbench; B=build/$QNX_ARCH/lmbench; mkdir -p $B
```

```bash
do_build() { cp -a "$S/." "$B/" && cd "$B/src" && \
  env CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CFLAGS="-O2 -D_QNX_SOURCE -DNO_RPC -DNO_PORTMAPPER -DRUSAGE -DSYS5" LDLIBS="-lm -lsocket" PTHREAD_LIBS="" OS=qnx \
      AR="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" RANLIB="$QNX_HOST/usr/bin/nto${QNX_CPU}-ranlib" ../scripts/build all; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/lib/lmbench/bin" && cp -a "$B/bin/qnx/." "$STAGE$PREFIX/lib/lmbench/bin/" && \
  cp -a "$B/scripts" "$STAGE$PREFIX/lib/lmbench/"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- lmbench builds through its scripts/build wrapper, which takes the compiler and an OS name for the output directory (bin/<OS>).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH lmbench`.

### 5. What it installs

106 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/lmbench` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa lmbench
ls dists/8.0/*/ | grep '^lmbench-'
```

This writes `lmbench-<ver>-r0.tar.gz` (run time) and `lmbench-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add lmbench
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/lmbench-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/lmbench-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_lmbench`](../../tests/src/) links lmbench the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-lmbench`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-lmbench
/usr/local/bin/qnx-port-tests/test_lmbench
```

```
== lmbench ==
  ok   ...
PASS lmbench
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_lmbench` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_lmbench`](../../tests/src/) -- a program linked against lmbench the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lmbench 3.0_git20260929-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/lmbench-3.0_git20260929-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lmbench 3.0_git20260929-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/lmbench-3.0_git20260929-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lmbench 3.0_git20260929-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/lmbench-3.0_git20260929-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lmbench 3.0_git20260929-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/lmbench-3.0_git20260929-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.0-git-20260929**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add lmbench=<version>`), sources in `ported/lmbench/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates lmbench`; port one with `scripts/port-update lmbench`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.0-git-20260929; GIT=https://github.com/intel/lmbench.git; COMMIT=27b43aeec9dc0bb45f420388c78e42dde4c23bf0
BUILDSYS=custom
# lmbench builds through its scripts/build wrapper, which takes the compiler
# and an OS name for the output directory (bin/<OS>).
do_build() { cp -a "$S/." "$B/" && cd "$B/src" && \
  env CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CFLAGS="-O2 -D_QNX_SOURCE -DNO_RPC -DNO_PORTMAPPER -DRUSAGE -DSYS5" LDLIBS="-lm -lsocket" PTHREAD_LIBS="" OS=qnx \
      AR="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" RANLIB="$QNX_HOST/usr/bin/nto${QNX_CPU}-ranlib" ../scripts/build all; }
do_install() { install -d "$STAGE$PREFIX/lib/lmbench/bin" && cp -a "$B/bin/qnx/." "$STAGE$PREFIX/lib/lmbench/bin/" && \
  cp -a "$B/scripts" "$STAGE$PREFIX/lib/lmbench/"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
