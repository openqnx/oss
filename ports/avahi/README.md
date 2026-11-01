# avahi 0.9-rc3

> Multicast/unicast DNS-SD framework (mDNS daemon, client library and tools)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/avahi/avahi.git (tag `v0.9-rc3`) |
| Build system | GNU Autotools |
| QNX patches | 4 |
| Build dependencies | [`dbus`](../dbus/) [`expat`](../expat/) [`libdaemon`](../libdaemon/) |
| Run-time dependencies | [`dbus`](../dbus/) [`expat`](../expat/) [`libdaemon`](../libdaemon/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_avahi`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `avahi` (programs, shared libraries, run-time data) and `avahi-dev` (headers, pkg-config/CMake files, static libraries; needs `avahi`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`avahi`](https://openqnx.github.io/oss/package/x86_64/avahi.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/avahi.html)) · [`avahi-dev`](https://openqnx.github.io/oss/package/x86_64/avahi-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/avahi-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add avahi` (dependencies come along); one version: `apk add avahi=<version>` | `apk del avahi` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install avahi` (dependencies first) | `oss-get remove avahi` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`avahi-dev` the same way, by its own name (`apk add avahi-dev`, `oss-get install avahi-dev`, by hand with its own file and list `avahi-dev.list`); remove it before `avahi`, which it needs.

```sh
# on the QNX target, as root
apk add avahi avahi-dev
apk add qnx-test-avahi && /usr/local/bin/qnx-port-tests/test_avahi    # its on-target test, from tests-8.0-<arch>
apk del avahi avahi-dev qnx-test-avahi
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/avahi.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/avahi.list
rm var/lib/manual/avahi.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild avahi` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (3 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild dbus expat libdaemon
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.9-rc3 --recurse-submodules --shallow-submodules \
    https://github.com/avahi/avahi.git src/avahi
```

### 3. Apply the QNX patches

4 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/avahi < ports/avahi/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/avahi < ports/avahi/patches/0002-nostdinc-builddir-config-h.patch
patch -p1 -d src/avahi < ports/avahi/patches/0003-no-drop-root-runtime-dir.patch
patch -p1 -d src/avahi < ports/avahi/patches/0004-sigint-watch-pollin-bits.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — qnx suppport

> Imported from qnx-ports/avahi (branch qnx-0.9.x, db5a8e5..7bf7222); original authorship kept.
>
> ---
>  avahi-common/Makefile.am                  | 4 ++++
>  avahi-compat-howl/include/salt/platform.h | 6 +++++-
>  avahi-daemon/main.c                       | 4 ++++
>  avahi-utils/sigint.c                      | 4 ++++
>  4 files changed, 17 insertions(+), 1 deletion(-)

Touches: `avahi-common/Makefile.am`, `avahi-compat-howl/include/salt/platform.h`, `avahi-daemon/main.c`, `avahi-utils/sigint.c` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-nostdinc-builddir-config-h.patch`](patches/0002-nostdinc-builddir-config-h.patch)** — avahi-common: find config.h in an out-of-tree build with nostdinc

> qnx-ports' patch 0001 sets AUTOMAKE_OPTIONS = nostdinc in avahi-common, so
> that avahi-common/malloc.h no longer shadows the system <malloc.h> that
> QNX's <stdlib.h> includes. nostdinc also drops -I$(top_builddir), which is
> where config.h is in an out-of-tree build (the fork builds in the source
> tree, where -I$(top_srcdir) happens to find it). Add it back explicitly.

Touches: `avahi-common/Makefile.am` · Upstream status: QNX-specific

**[`0003-no-drop-root-runtime-dir.patch`](patches/0003-no-drop-root-runtime-dir.patch)** — avahi-daemon: --no-drop-root does not need an avahi user for its runtime directory

> avahi-daemon creates its runtime directory (PID file, client socket) owned
> by the avahi user and group even with --no-drop-root, and fails when that
> user does not exist -- which is the case on a stock QNX system, so the
> daemon could not be started at all without creating accounts first. With
> --no-drop-root the daemon keeps the identity it was started with: create
> the directory (and its missing parents: under a /usr/local prefix var/run
> may not exist yet) owned by that user instead.

Touches: `avahi-daemon/main.c` · Upstream status: QNX-specific

**[`0004-sigint-watch-pollin-bits.patch`](patches/0004-sigint-watch-pollin-bits.patch)** — avahi-utils: accept any POLLIN bit in the signal pipe watch

> QNX defines POLLIN as POLLRDNORM|POLLRDBAND, and poll() reports a pipe
> with data as POLLRDNORM alone. The tools' SIGINT/SIGTERM pipe watch asserted
> that the event was exactly AVAHI_WATCH_IN (== POLLIN), so avahi-browse,
> avahi-publish, avahi-resolve ... aborted with a core dump instead of
> exiting when interrupted or terminated. Test the bit instead.

Touches: `avahi-utils/sigint.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/avahi && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/avahi && cd build/$QNX_ARCH/avahi
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/avahi/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --with-distro=none \
    --disable-nls \
    --disable-glib \
    --disable-gobject \
    --disable-libevent \
    --disable-qt3 \
    --disable-qt4 \
    --disable-qt5 \
    --disable-gtk \
    --disable-gtk3 \
    --disable-python \
    --disable-pygobject \
    --disable-python-dbus \
    --disable-mono \
    --disable-monodoc \
    --disable-gdbm \
    --disable-dbm \
    --disable-autoipd \
    --disable-libsystemd \
    --with-systemdsystemunitdir=no \
    --disable-manpages \
    --disable-xmltoman \
    --disable-doxygen-doc \
    --disable-tests \
    --enable-compat-libdns_sd \
    --disable-compat-howl \
    --with-xml=expat \
    --enable-dbus \
    --enable-libdaemon \
    # \
    The \
    system \
    bus \
    where \
    the \
    dbus \
    port \
    puts \
    it \
    (its \
    system_bus_default_address). \
    --with-dbus-system-address=unix:path=/var/local/run/dbus/system_bus_socket \
    # \
    A \
    stock \
    QNX \
    target \
    has \
    no \
    avahi \
    account: \
    run \
    the \
    daemon \
    with \
    # \
    --no-drop-root \
    (as \
    root), \
    or \
    create \
    the \
    account. \
    --with-avahi-user=avahi \
    --with-avahi-group=avahi
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Avahi mDNS/DNS-SD: avahi-daemon, libavahi-common/-core/-client, the avahi-browse/-publish/-resolve tools and the libdns_sd compatibility library. 0.9-rc3 with qnx-ports' QNX support (patches/0001: SA_RESTART, <stdint.h> types for the howl headers, avahi-common/malloc.h no longer shadowing <malloc.h>), the version that fork carries; 0002 makes 0001 work in an out-of-tree build, 0003 lets "avahi-daemon --no-drop-root" run without an avahi account (a stock QNX system has none), 0004 stops the client tools aborting on SIGINT/SIGTERM (QNX's POLLIN is two bits).
- QNX's network stack is FreeBSD's: interfaces and addresses come from the PF_ROUTE socket (avahi-core/iface-pfroute.c; configure picks it from <net/route.h>, as on the BSDs). Netlink does not exist. Turned off: the GUI and language bindings (GTK, Qt, Python, Mono), the glib/libevent main-loop adapters, gdbm (the service-type name database avahi-browse uses for "human readable" names; the tools print the raw type), avahi-autoipd (IPv4LL through Linux packet sockets), systemd, and the man pages (xmltoman). --with-distro=none: no init scripts.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH avahi`.

### 5. What it installs

67 files under `/usr/local`: 8 shared-library entries, 11 programs, 24 headers.

```
/usr/local/lib/libavahi-client.so
/usr/local/lib/libavahi-client.so.5
/usr/local/lib/libavahi-common.so
/usr/local/lib/libavahi-common.so.8
/usr/local/lib/libavahi-core.so
/usr/local/lib/libavahi-core.so.8
/usr/local/lib/libdns_sd.so
/usr/local/lib/libdns_sd.so.1
/usr/local/bin/avahi-browse
/usr/local/bin/avahi-browse-domains
/usr/local/bin/avahi-publish
/usr/local/bin/avahi-publish-address
/usr/local/bin/avahi-publish-service
/usr/local/bin/avahi-resolve
/usr/local/bin/avahi-resolve-address
/usr/local/bin/avahi-resolve-host-name
...
```

The complete list is `stage/<arch>/.manifest/avahi` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa avahi
ls dists/8.0/*/ | grep '^avahi-'
```

This writes `avahi-<ver>-r0.tar.gz` (run time) and `avahi-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add avahi
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/avahi-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/avahi-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_avahi`](../../tests/src/) links avahi the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-avahi`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-avahi
/usr/local/bin/qnx-port-tests/test_avahi
```

```
== avahi ==
  ok   ...
PASS avahi
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_avahi` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_avahi`](../../tests/src/) -- a program linked against avahi the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | avahi 0.9_rc3-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/avahi-0.9_rc3-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | avahi 0.9_rc3-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/avahi-0.9_rc3-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.9-rc3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add avahi=<version>`), sources in `ported/avahi/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates avahi`; port one with `scripts/port-update avahi`.

<details><summary>recipe.sh</summary>

```bash
# Avahi mDNS/DNS-SD: avahi-daemon, libavahi-common/-core/-client, the
# avahi-browse/-publish/-resolve tools and the libdns_sd compatibility
# library. 0.9-rc3 with qnx-ports' QNX support (patches/0001: SA_RESTART,
# <stdint.h> types for the howl headers, avahi-common/malloc.h no longer
# shadowing <malloc.h>), the version that fork carries; 0002 makes 0001 work
# in an out-of-tree build, 0003 lets "avahi-daemon --no-drop-root" run
# without an avahi account (a stock QNX system has none), 0004 stops the
# client tools aborting on SIGINT/SIGTERM (QNX's POLLIN is two bits).
VERSION=0.9-rc3; GIT=https://github.com/avahi/avahi.git; REF=v0.9-rc3
DEPS="dbus expat libdaemon"
BUILDSYS=autotools
# QNX's network stack is FreeBSD's: interfaces and addresses come from the
# PF_ROUTE socket (avahi-core/iface-pfroute.c; configure picks it from
# <net/route.h>, as on the BSDs). Netlink does not exist.
# Turned off: the GUI and language bindings (GTK, Qt, Python, Mono), the
# glib/libevent main-loop adapters, gdbm (the service-type name database
# avahi-browse uses for "human readable" names; the tools print the raw
# type), avahi-autoipd (IPv4LL through Linux packet sockets), systemd, and the
# man pages (xmltoman). --with-distro=none: no init scripts.
CONF_ARGS_EXTRA=(
  --enable-shared --disable-static --with-distro=none --disable-nls
  --disable-glib --disable-gobject --disable-libevent --disable-qt3 --disable-qt4 --disable-qt5
  --disable-gtk --disable-gtk3 --disable-python --disable-pygobject --disable-python-dbus
  --disable-mono --disable-monodoc --disable-gdbm --disable-dbm --disable-autoipd
  --disable-libsystemd --with-systemdsystemunitdir=no --disable-manpages --disable-xmltoman
  --disable-doxygen-doc --disable-tests --enable-compat-libdns_sd --disable-compat-howl
  --with-xml=expat --enable-dbus --enable-libdaemon
  # The system bus where the dbus port puts it (its system_bus_default_address).
  --with-dbus-system-address=unix:path=/var/local/run/dbus/system_bus_socket
  # A stock QNX target has no "avahi" account: run the daemon with
  # --no-drop-root (as root), or create the account.
  --with-avahi-user=avahi --with-avahi-group=avahi
)
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
