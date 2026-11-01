# lldpd 1.0.22

> Implementation of IEEE 802.1ab (LLDP)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/lldpd/lldpd.git (tag `1.0.22`) |
| Build system | GNU Autotools |
| QNX patches | 8 |
| Build dependencies | [`libevent`](../libevent/) |
| Run-time dependencies | [`libevent`](../libevent/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_lldpd`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `lldpd` (programs, shared libraries, run-time data) and `lldpd-dev` (headers, pkg-config/CMake files, static libraries; needs `lldpd`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`lldpd`](https://openqnx.github.io/oss/package/x86_64/lldpd.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lldpd.html)) · [`lldpd-dev`](https://openqnx.github.io/oss/package/x86_64/lldpd-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lldpd-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add lldpd` (dependencies come along); one version: `apk add lldpd=<version>` | `apk del lldpd` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install lldpd` (dependencies first) | `oss-get remove lldpd` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`lldpd-dev` the same way, by its own name (`apk add lldpd-dev`, `oss-get install lldpd-dev`, by hand with its own file and list `lldpd-dev.list`); remove it before `lldpd`, which it needs.

```sh
# on the QNX target, as root
apk add lldpd lldpd-dev
apk add qnx-test-lldpd && /usr/local/bin/qnx-port-tests/test_lldpd    # its on-target test, from tests-8.0-<arch>
apk del lldpd lldpd-dev qnx-test-lldpd
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/lldpd.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/lldpd.list
rm var/lib/manual/lldpd.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild lldpd` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (2 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libevent
```

<details><summary>Full build chain (2)</summary>

[`libevent`](../libevent/) [`openssl`](../openssl/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.0.22 --recurse-submodules --shallow-submodules \
    https://github.com/lldpd/lldpd.git src/lldpd
```

### 3. Apply the QNX patches

8 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/lldpd < ports/lldpd/patches/0001-recognize-qnx-build-the-bsd-based-network-backend.patch
patch -p1 -d src/lldpd < ports/lldpd/patches/0002-open-the-ioctl-socket-before-the-privsep-chroot.patch
patch -p1 -d src/lldpd < ports/lldpd/patches/0003-proxy-getifaddrs-through-the-privileged-monitor.patch
patch -p1 -d src/lldpd < ports/lldpd/patches/0004-open-the-route-socket-before-the-privsep-chroot.patch
patch -p1 -d src/lldpd < ports/lldpd/patches/0005-use-the-single-dev-bpf-clone-device.patch
patch -p1 -d src/lldpd < ports/lldpd/patches/0006-treat-enoent-as-a-stale-control-socket.patch
patch -p1 -d src/lldpd < ports/lldpd/patches/0007-qnx-no-sa-restart.patch
patch -p1 -d src/lldpd < ports/lldpd/patches/0008-noprivsep-lock-mode.patch
```

**[`0001-recognize-qnx-build-the-bsd-based-network-backend.patch`](patches/0001-recognize-qnx-build-the-bsd-based-network-backend.patch)** — recognize QNX, build the BSD-based network backend

> Imported from qnx-ports/lldpd (9c1458ec9d91, branch qnx-1.0.22, based on upstream 1.0.22);
> original authorship kept.
>
> ---
>  configure.ac                | 11 +++++++++++
>  m4/os.m4                    |  1 +
>  src/daemon/Makefile.am      |  8 ++++++++
>  src/daemon/interfaces-bsd.c | 10 +++++-----
>  src/daemon/priv.c           |  4 ++--
>  5 files changed, 27 insertions(+), 7 deletions(-)

Touches: `configure.ac`, `m4/os.m4`, `src/daemon/Makefile.am`, `src/daemon/interfaces-bsd.c`, `src/daemon/priv.c` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-open-the-ioctl-socket-before-the-privsep-chroot.patch`](patches/0002-open-the-ioctl-socket-before-the-privsep-chroot.patch)** — open the ioctl socket before the privsep chroot

> Imported from qnx-ports/lldpd (4cba3a3a6ec5, branch qnx-1.0.22, based on upstream 1.0.22);
> original authorship kept.
>
> ---
>  src/daemon/lldpd.c | 17 +++++++++++++----
>  1 file changed, 13 insertions(+), 4 deletions(-)

Touches: `src/daemon/lldpd.c` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0003-proxy-getifaddrs-through-the-privileged-monitor.patch`](patches/0003-proxy-getifaddrs-through-the-privileged-monitor.patch)** — proxy getifaddrs() through the privileged monitor

> Imported from qnx-ports/lldpd (a77a8b98e471, branch qnx-1.0.22, based on upstream 1.0.22);
> original authorship kept.
>
> ---
>  src/daemon/interfaces-bsd.c |   8 ++
>  src/daemon/lldpd.h          |   8 ++
>  src/daemon/priv.c           | 165 +++++++++++++++++++++++++++++++++++-
>  3 files changed, 180 insertions(+), 1 deletion(-)

Touches: `src/daemon/interfaces-bsd.c`, `src/daemon/lldpd.h`, `src/daemon/priv.c` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0004-open-the-route-socket-before-the-privsep-chroot.patch`](patches/0004-open-the-route-socket-before-the-privsep-chroot.patch)** — open the route socket before the privsep chroot

> Imported from qnx-ports/lldpd (686d44cef719, branch qnx-1.0.22, based on upstream 1.0.22);
> original authorship kept.
>
> ---
>  src/daemon/interfaces-bsd.c | 12 ++++++++++++
>  src/daemon/lldpd.c          | 19 +++++++++++++++++++
>  src/daemon/lldpd.h          |  4 ++++
>  3 files changed, 35 insertions(+)

Touches: `src/daemon/interfaces-bsd.c`, `src/daemon/lldpd.c`, `src/daemon/lldpd.h` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0005-use-the-single-dev-bpf-clone-device.patch`](patches/0005-use-the-single-dev-bpf-clone-device.patch)** — use the single /dev/bpf clone device

> Imported from qnx-ports/lldpd (3b38719207a7, branch qnx-1.0.22, based on upstream 1.0.22);
> original authorship kept.
>
> ---
>  src/daemon/priv-bsd.c | 10 +++++++++-
>  1 file changed, 9 insertions(+), 1 deletion(-)

Touches: `src/daemon/priv-bsd.c` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0006-treat-enoent-as-a-stale-control-socket.patch`](patches/0006-treat-enoent-as-a-stale-control-socket.patch)** — treat ENOENT as a stale-socket signal in the

> control-socket retry loop
>
>
> Imported from qnx-ports/lldpd (ac20728ae287, branch qnx-1.0.22, based on upstream 1.0.22);
> original authorship kept.
>
> ---
>  src/daemon/lldpd.c | 16 +++++++++++++++-
>  1 file changed, 15 insertions(+), 1 deletion(-)

Touches: `src/daemon/lldpd.c` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0007-qnx-no-sa-restart.patch`](patches/0007-qnx-no-sa-restart.patch)** — Define SA_RESTART to 0 where the system lacks it (QNX).

> QNX 8.0 does not implement SA_RESTART: <signal.h> keeps the flag commented
> out ("not supported yet"), so priv.c does not compile. Its SIGCHLD and
> forwarding handlers only reap children or pass the signal on; the privsep
> channel's must_read()/must_write() already retry on EINTR and libevent's
> loop does too, so defining the flag to 0 where it is missing keeps the
> behaviour correct.

Touches: `src/daemon/priv.c` · Upstream status: QNX-specific

**[`0008-noprivsep-lock-mode.patch`](patches/0008-noprivsep-lock-mode.patch)** — Create the control lock file 0600 when built without privilege separation.

> lldpd creates the control socket's lock file with mode 0000 and, only when
> built with privilege separation, chowns it to the privsep user and chmods
> it 0770. Built with --disable-privsep (as here: a QNX image has no _lldpd
> user) the file keeps mode 0000, and every lldpcli "configure ..." command,
> including those lldpd itself runs from lldpd.conf at start, fails "cannot
> open lock ...: Permission denied" unless it runs as root. Create the file
> 0600 in that build.

Touches: `src/daemon/lldpd.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/lldpd && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/lldpd && cd build/$QNX_ARCH/lldpd
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/lldpd/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-privsep \
    --runstatedir=/var/run \
    --localstatedir=/var \
    --without-snmp \
    --without-xml \
    --without-readline \
    --without-seccomp \
    --with-libbsd=no \
    --with-embedded-libevent=no \
    --disable-static \
    --enable-shared \
    --enable-min-macro
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- lldpd from upstream 1.0.22 with QNX's changes (qnx-ports/lldpd, branch qnx-1.0.22): QNX is recognised as an OS and gets the BSD network backend (route socket, BPF on the single /dev/bpf clone device, getifaddrs()). From git rather than the release tarball: patch 0001 changes configure.ac, m4/os.m4 and Makefile.am, so configure has to be regenerated (autogen.sh).
- --disable-privsep: privilege separation needs a dedicated "_lldpd" user and an empty chroot directory, and a QNX image has neither (a package cannot create users). Without it lldpd runs in one process and emulates the monitor in-process; QNX's patches (sockets opened early, getifaddrs() through the monitor) work the same in that mode. --runstatedir=/var/run: the control socket and pid file go where QNX keeps run-time state, not under /usr/local. --without-snmp/xml/readline/seccomp: optional features with no QNX dependency staged (net-snmp, libxml2 output) or Linux-only (seccomp); lldpcli still has its own command line and JSON/keyvalue output. --with-libbsd=no: libbsd is not ported; lldpd carries its own copies of the few BSD functions it needs (QNX's libc has strlcpy/strlcat already). --enable-min-macro (added by QNX's patch 0001): lldpd uses MIN() from <sys/param.h> as the BSDs and glibc define it; QNX's headers do not, and the calls ended up as undefined references to a function MIN.
- The version comes from `git describe`, which reports "1.0.22-dirty" for a patched checkout; a release tarball carries .dist-version instead.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH lldpd`.

### 5. What it installs

20 files under `/usr/local`: 2 shared-library entries, 3 programs, 3 headers.

```
/usr/local/lib/liblldpctl.so
/usr/local/lib/liblldpctl.so.14
/usr/local/sbin/lldpcli
/usr/local/sbin/lldpctl
/usr/local/sbin/lldpd
```

The complete list is `stage/<arch>/.manifest/lldpd` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa lldpd
ls dists/8.0/*/ | grep '^lldpd-'
```

This writes `lldpd-<ver>-r0.tar.gz` (run time) and `lldpd-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add lldpd
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/lldpd-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/lldpd-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_lldpd`](../../tests/src/) links lldpd the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-lldpd`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-lldpd
/usr/local/bin/qnx-port-tests/test_lldpd
```

```
== lldpd ==
  ok   ...
PASS lldpd
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_lldpd` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_lldpd`](../../tests/src/) -- a program linked against lldpd the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lldpd 1.0.22-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/lldpd-1.0.22-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lldpd 1.0.22-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/lldpd-1.0.22-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.0.22**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add lldpd=<version>`), sources in `ported/lldpd/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates lldpd`; port one with `scripts/port-update lldpd`.

<details><summary>recipe.sh</summary>

```bash
# lldpd from upstream 1.0.22 with QNX's changes (qnx-ports/lldpd, branch
# qnx-1.0.22): QNX is recognised as an OS and gets the BSD network backend
# (route socket, BPF on the single /dev/bpf clone device, getifaddrs()).
# From git rather than the release tarball: patch 0001 changes configure.ac,
# m4/os.m4 and Makefile.am, so configure has to be regenerated (autogen.sh).
VERSION=1.0.22; GIT=https://github.com/lldpd/lldpd.git; REF=1.0.22
BUILDSYS=autotools
DEPS="libevent"
# --disable-privsep: privilege separation needs a dedicated "_lldpd" user and
#   an empty chroot directory, and a QNX image has neither (a package cannot
#   create users). Without it lldpd runs in one process and emulates the
#   monitor in-process; QNX's patches (sockets opened early, getifaddrs()
#   through the monitor) work the same in that mode.
# --runstatedir=/var/run: the control socket and pid file go where QNX keeps
#   run-time state, not under /usr/local.
# --without-snmp/xml/readline/seccomp: optional features with no QNX
#   dependency staged (net-snmp, libxml2 output) or Linux-only (seccomp);
#   lldpcli still has its own command line and JSON/keyvalue output.
# --with-libbsd=no: libbsd is not ported; lldpd carries its own copies of the
#   few BSD functions it needs (QNX's libc has strlcpy/strlcat already).
# --enable-min-macro (added by QNX's patch 0001): lldpd uses MIN() from
#   <sys/param.h> as the BSDs and glibc define it; QNX's headers do not, and
#   the calls ended up as undefined references to a function MIN.
CONF_ARGS_EXTRA=( --disable-privsep --runstatedir=/var/run --localstatedir=/var
                  --without-snmp --without-xml --without-readline --without-seccomp
                  --with-libbsd=no --with-embedded-libevent=no
                  --disable-static --enable-shared --enable-min-macro )
# The version comes from `git describe`, which reports "1.0.22-dirty" for a
# patched checkout; a release tarball carries .dist-version instead.
do_patch() { printf '%s' "$VERSION" > "$SRCROOT/.dist-version"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
