# chrony 4.8-git-20260226

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/mlichvar/chrony.git |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install chrony
oss-get install chrony-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild chrony` performs exactly the same steps (2–5) in one command.

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
    https://github.com/mlichvar/chrony.git src/chrony
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/chrony < ports/chrony/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/chrony < ports/chrony/patches/0002-qnx-qcc-compatible-cflags.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add qnx support

> Imported from qnx-ports/chrony (d622b222a9ac...master); original authorship kept.
>
>
> This pull request introduces source code changes to add QNX support for chrony 4.8
> ---
>  Makefile.in            |   8 -
>  adjtime_qnx.h          | 154 ++++++++++++++++
>  client.c               |   4 +-
>  configure              |  17 ++
>  socket.c               |   6 +
>  sys.c                  |   9 +
>  sys_qnx.c              | 399 +++++++++++++++++++++++++++++++++++++++++
>  sys_qnx.h              |  41 +++++
>  sysincl.h              |   2 +
>  test/unit/quantiles.c  |   1 +
>  test/unit/samplefilt.c |   1 +
>  test/unit/socket.c     |   1 +
>  12 files changed, 633 insertions(+), 10 deletions(-)
>  create mode 100644 adjtime_qnx.h
>  create mode 100644 sys_qnx.c
>  create mode 100644 sys_qnx.h

Touches: `Makefile.in`, `adjtime_qnx.h`, `client.c`, `configure`, `socket.c`, `sys.c` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-qcc-compatible-cflags.patch`](patches/0002-qnx-qcc-compatible-cflags.patch)** — Use compiler flags qcc accepts in the QNX configure case.

> The QNX case sets CFLAGS with "--param=ssp-buffer-size=4", which qcc (the
> QNX compiler driver) rejects as an unknown option -- configure then reports
> that the C compiler cannot create executables. -fstack-protector-strong
> already selects a sensible buffer size. Also drop -I/usr/include/p11-kit-1,
> a path of the build host that has no business in a cross build.

Touches: `configure` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/chrony; B=build/$QNX_ARCH/chrony; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/" && cd "$B" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
  CPPFLAGS="-I$STAGE$PREFIX/include" LDFLAGS="-L$STAGE$PREFIX/lib -lsocket" \
  ./configure --host-system=QNX --host-release=8.0.0 --host-machine="$QNX_CPU" --prefix="$PREFIX" --without-nettle --without-gnutls --without-nss --without-tomcrypt \
     --disable-readline --without-editline --without-seccomp --without-libcap --disable-privdrop --disable-pps; }
```

```bash
do_build() { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- chrony from upstream at the commit qnx-ports' QNX support is based on; the patch adds a QNX system driver (sys_qnx.c: clock slewing through ClockAdjust()) and a QNX case in configure.
- chrony's configure is hand-written (not autoconf) and picks the system driver from the OS name it is given.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH chrony`.

### 5. What it installs

2 files under `/usr/local`, 2 programs.

```
/usr/local/bin/chronyc
/usr/local/sbin/chronyd
```

The complete list is `stage/<arch>/.manifest/chrony` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH chrony
ls repo/8.0/*/ | grep '^chrony-'
```

This writes `chrony-<ver>-r0.tar.gz` (run time) and `chrony-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install chrony
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/chrony-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/chrony-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# chrony from upstream at the commit qnx-ports' QNX support is based on; the
# patch adds a QNX system driver (sys_qnx.c: clock slewing through
# ClockAdjust()) and a QNX case in configure.
VERSION=4.8-git-20260226; GIT=https://github.com/mlichvar/chrony.git; COMMIT=d622b222a9acbda3a4c842beeb593e7075c32bec
BUILDSYS=custom
# chrony's configure is hand-written (not autoconf) and picks the system
# driver from the OS name it is given.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
  CPPFLAGS="-I$STAGE$PREFIX/include" LDFLAGS="-L$STAGE$PREFIX/lib -lsocket" \
  ./configure --host-system=QNX --host-release=8.0.0 --host-machine="$QNX_CPU" --prefix="$PREFIX" --without-nettle --without-gnutls --without-nss --without-tomcrypt \
     --disable-readline --without-editline --without-seccomp --without-libcap --disable-privdrop --disable-pps; }
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
