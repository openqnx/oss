# htop 3.5.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/htop-dev/htop.git (tag `3.5.0`) |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`ncurses`](../ncurses/) |
| Run-time dependencies | [`ncurses`](../ncurses/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_monitors`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add htop
apk add htop-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-monitors && /usr/local/bin/qnx-port-tests/test_monitors    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install htop`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild htop` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild ncurses
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.5.0 --recurse-submodules --shallow-submodules \
    https://github.com/htop-dev/htop.git src/htop
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/htop < ports/htop/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — htop on QNX, from QNX's port (github.com/qnx-ports/htop, branch qnx-3.5.0):

> a QNX platform for htop -- qnx/: the process table read from /proc (each
> process's debug info through devctl), per-CPU and memory figures from the
> system page, process fields QNX has -- and its wiring into configure.ac and
> Makefile.am.

Touches: `Makefile.am`, `configure.ac`, `qnx/Platform.c`, `qnx/Platform.h`, `qnx/ProcessField.h`, `qnx/QNXMachine.c` … · Upstream status: Inappropriate [QNX's port, kept as QNX published it]

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/htop && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/htop && cd build/$QNX_ARCH/htop
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/htop/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-unicode \
    --disable-sensors \
    --disable-capabilities
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- htop: the interactive process viewer, with QNX's QNX platform (patches/0001, from github.com/qnx-ports/htop: processes from /proc, CPU and memory from QNX's system page).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH htop`.

### 5. What it installs

5 files under `/usr/local`, 1 programs.

```
/usr/local/bin/htop
```

The complete list is `stage/<arch>/.manifest/htop` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa htop
ls dists/8.0/*/ | grep '^htop-'
```

This writes `htop-<ver>-r0.tar.gz` (run time) and `htop-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add htop
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/htop-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/htop-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_monitors`](../../tests/src/) links htop the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-monitors`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-monitors
/usr/local/bin/qnx-port-tests/test_monitors
```

```
== htop ==
  ok   ...
PASS htop
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_monitors` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_monitors`](../../tests/src/) -- a program linked against htop the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - htop drew a screen
   - htop: header and meters
   - htop: lists sshd (QNX process table)
   - htop: lists io-sock
   - htop exits on q
   - btop drew a screen
   - btop: cpu, mem, proc boxes
   - btop: lists this target's processes
   - btop exits on q

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | btop 1.4.6-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | htop 3.5.0-r0 | ❌ FAIL | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | btop 1.4.6-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | htop 3.5.0-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **3.5.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add htop=<version>`), sources in `ported/htop/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates htop`; port one with `scripts/port-update htop`.

<details><summary>recipe.sh</summary>

```bash
# htop: the interactive process viewer, with QNX's QNX platform (patches/0001,
# from github.com/qnx-ports/htop: processes from /proc, CPU and memory from
# QNX's system page).
VERSION=3.5.0; GIT=https://github.com/htop-dev/htop.git; REF=3.5.0
DEPS="ncurses"
BUILDSYS=autotools
do_patch() { cd "$SRCROOT" && ./autogen.sh; }
CONF_ARGS_EXTRA=( --enable-unicode --disable-sensors --disable-capabilities )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
