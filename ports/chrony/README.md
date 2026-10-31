# chrony 4.8-git-20260226

> NTP client and server for synchronizing the system clock

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/mlichvar/chrony.git |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_chrony`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add chrony
apk add chrony-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-chrony && /usr/local/bin/qnx-port-tests/test_chrony    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install chrony`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa chrony
ls dists/8.0/*/ | grep '^chrony-'
```

This writes `chrony-<ver>-r0.tar.gz` (run time) and `chrony-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add chrony
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/chrony-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/chrony-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_chrony`](../../tests/src/) links chrony the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-chrony`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-chrony
/usr/local/bin/qnx-port-tests/test_chrony
```

```
== chrony ==
  ok   ...
PASS chrony
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_chrony` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_chrony`](../../tests/src/) -- a program linked against chrony the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | chrony 4.8_git20260226-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/chrony-4.8_git20260226-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | chrony 4.8_git20260226-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/chrony-4.8_git20260226-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | chrony 4.8_git20260226-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/chrony-4.8_git20260226-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | chrony 4.8_git20260226-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/chrony-4.8_git20260226-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.8-git-20260226**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add chrony=<version>`), sources in `ported/chrony/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates chrony`; port one with `scripts/port-update chrony`.

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
