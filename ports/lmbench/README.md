# lmbench 3.0-git-20260929

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/intel/lmbench.git |
| Build system | custom (see recipe) |
| QNX patches | 5 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install lmbench
oss-get install lmbench-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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
./scripts/qnxpkg -a $QNX_ARCH lmbench
ls repo/8.0/*/ | grep '^lmbench-'
```

This writes `lmbench-<ver>-r0.tar.gz` (run time) and `lmbench-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install lmbench
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/lmbench-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/lmbench-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

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
