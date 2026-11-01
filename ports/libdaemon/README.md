# libdaemon 0.14

> Lightweight C library that eases the writing of UNIX daemons

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | http://0pointer.de/lennart/projects/libdaemon/libdaemon-0.14.tar.gz |
| SHA-256 | `fd23eb5f6f986dcc7e708307355ba3289abe03cc381fc47a80bca4a50aa6b834` |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libdaemon`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libdaemon` (programs, shared libraries, run-time data) and `libdaemon-dev` (headers, pkg-config/CMake files, static libraries; needs `libdaemon`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libdaemon`](https://openqnx.github.io/oss/package/x86_64/libdaemon.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libdaemon.html)) · [`libdaemon-dev`](https://openqnx.github.io/oss/package/x86_64/libdaemon-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libdaemon-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libdaemon` (dependencies come along); one version: `apk add libdaemon=<version>` | `apk del libdaemon` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libdaemon` (dependencies first) | `oss-get remove libdaemon` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libdaemon-dev` the same way, by its own name (`apk add libdaemon-dev`, `oss-get install libdaemon-dev`, by hand with its own file and list `libdaemon-dev.list`); remove it before `libdaemon`, which it needs.

```sh
# on the QNX target, as root
apk add libdaemon libdaemon-dev
apk add qnx-test-libdaemon && /usr/local/bin/qnx-port-tests/test_libdaemon    # its on-target test, from tests-8.0-<arch>
apk del libdaemon libdaemon-dev qnx-test-libdaemon
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libdaemon.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libdaemon.list
rm var/lib/manual/libdaemon.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libdaemon` performs exactly the same steps (2–5) in one command.

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
URL=http://0pointer.de/lennart/projects/libdaemon/libdaemon-0.14.tar.gz
mkdir -p dist src/libdaemon
curl -fL -o dist/$(basename $URL) $URL
echo "fd23eb5f6f986dcc7e708307355ba3289abe03cc381fc47a80bca4a50aa6b834  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libdaemon --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libdaemon < ports/libdaemon/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/libdaemon < ports/libdaemon/patches/0002-fork-retry-eintr.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — initial qnx support

> Imported from qnx-ports/libdaemon (branch qnx-0.14, 8616602..ddad99d); original authorship kept.
>
> ---
>  examples/testd.c    | 2 +-
>  libdaemon/dexec.c   | 8 ++++++++
>  libdaemon/dfork.c   | 4 ++++
>  libdaemon/dsignal.c | 4 ++++
>  4 files changed, 17 insertions(+), 1 deletion(-)

Touches: `examples/testd.c`, `libdaemon/dexec.c`, `libdaemon/dfork.c`, `libdaemon/dsignal.c` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-fork-retry-eintr.patch`](patches/0002-fork-retry-eintr.patch)** — daemon_fork: retry interrupted system calls (no SA_RESTART on QNX)

> qnx-ports' patch 0001 defines SA_RESTART to 0 (QNX does not implement it),
> so the SIGCHLD handler daemon_fork() installs now interrupts the parent's
> waitpid() for the first child with EINTR, and daemon_fork() returned -1 in
> the parent although the daemon had started (avahi-daemon -D then reports
> that it failed to fork). Retry waitpid(), read(), write() and the select()
> in daemon_retval_wait() on EINTR, as daemon_exec() already does.

Touches: `libdaemon/dfork.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libdaemon && cd build/$QNX_ARCH/libdaemon
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
ac_cv_func_setpgrp_void=yes \
$OLDPWD/src/libdaemon/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-lynx \
    --disable-examples
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libdaemon 0.14 (the last release) with qnx-ports' QNX support (patches/0001: SA_RESTART, which QNX does not implement, defined to 0; <sys/select.h> for select()); patches/0002 then retries the system calls daemon_fork() had relied on SA_RESTART for (EINTR from its SIGCHLD handler made it return -1 in the parent of a daemon that had started). Used by avahi-daemon to daemonize, write its PID file and pass signals through a pipe.
- The 2012 tarball's config.sub predates the *-nto-qnx8.0.0 triples: autoreconf replaces the generated build system with the host's current one.
- No lynx/doxygen docs; the examples are only built, never installed. AC_FUNC_SETPGRP is a run-test; QNX has the POSIX setpgrp(void).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libdaemon`.

### 5. What it installs

12 files under `/usr/local`: 2 shared-library entries, 7 headers.

```
/usr/local/lib/libdaemon.so
/usr/local/lib/libdaemon.so.5
```

The complete list is `stage/<arch>/.manifest/libdaemon` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libdaemon
ls dists/8.0/*/ | grep '^libdaemon-'
```

This writes `libdaemon-<ver>-r0.tar.gz` (run time) and `libdaemon-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libdaemon
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libdaemon-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libdaemon-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libdaemon`](../../tests/src/) links libdaemon the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libdaemon`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libdaemon
/usr/local/bin/qnx-port-tests/test_libdaemon
```

```
== libdaemon ==
  ok   ...
PASS libdaemon
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libdaemon` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libdaemon`](../../tests/src/) -- a program linked against libdaemon the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - scratch directory
   - no daemon running before the test
   - daemon_retval_init
   - daemon_fork returns the daemon's PID in the parent
   - daemon reported ready (retval 42)
   - PID file names the daemon daemon_fork reported
   - the daemon runs in a session of its own (setsid)
   - daemon_pid_file_kill(SIGUSR1)
   - daemon handled SIGUSR1 through its signal pipe
   - daemon_pid_file_kill_wait(SIGTERM)
   - daemon process gone
   - daemon removed its PID file
   - daemon_signal_init(SIGCHLD)
   - daemon_exec: child exit status 7
   - pipe
   - daemon_nonblock(1) sets O_NONBLOCK
   - daemon_nonblock(0) clears it

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libdaemon 0.14-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libdaemon-0.14-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libdaemon 0.14-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libdaemon-0.14-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.14**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libdaemon=<version>`), sources in `ported/libdaemon/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libdaemon`; port one with `scripts/port-update libdaemon`.

<details><summary>recipe.sh</summary>

```bash
# libdaemon 0.14 (the last release) with qnx-ports' QNX support
# (patches/0001: SA_RESTART, which QNX does not implement, defined to 0;
# <sys/select.h> for select()); patches/0002 then retries the system calls
# daemon_fork() had relied on SA_RESTART for (EINTR from its SIGCHLD
# handler made it return -1 in the parent of a daemon that had started).
# Used by avahi-daemon to daemonize, write its PID file and pass signals
# through a pipe.
VERSION=0.14
TARBALL=http://0pointer.de/lennart/projects/libdaemon/libdaemon-0.14.tar.gz
SHA256=fd23eb5f6f986dcc7e708307355ba3289abe03cc381fc47a80bca4a50aa6b834
BUILDSYS=autotools
# The 2012 tarball's config.sub predates the *-nto-qnx8.0.0 triples:
# autoreconf replaces the generated build system with the host's current one.
do_patch() { ( cd "$SRCROOT" && autoreconf -fi ); }
# No lynx/doxygen docs; the examples are only built, never installed.
# AC_FUNC_SETPGRP is a run-test; QNX has the POSIX setpgrp(void).
CONF_ENV=( ac_cv_func_setpgrp_void=yes )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-lynx --disable-examples )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
