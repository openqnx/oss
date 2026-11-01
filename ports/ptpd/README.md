# ptpd 2.3.1

> Precision Time Protocol (IEEE 1588) daemon

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/ptpd/ptpd.git (tag `ptpd.2.3.1`) |
| Build system | GNU Autotools |
| QNX patches | 4 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_ptpd`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `ptpd` (programs, shared libraries, run-time data) and `ptpd-dev` (headers, pkg-config/CMake files, static libraries; needs `ptpd`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`ptpd`](https://openqnx.github.io/oss/package/x86_64/ptpd.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ptpd.html)) · [`ptpd-dev`](https://openqnx.github.io/oss/package/x86_64/ptpd-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ptpd-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add ptpd` (dependencies come along); one version: `apk add ptpd=<version>` | `apk del ptpd` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install ptpd` (dependencies first) | `oss-get remove ptpd` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`ptpd-dev` the same way, by its own name (`apk add ptpd-dev`, `oss-get install ptpd-dev`, by hand with its own file and list `ptpd-dev.list`); remove it before `ptpd`, which it needs.

```sh
# on the QNX target, as root
apk add ptpd ptpd-dev
apk add qnx-test-ptpd && /usr/local/bin/qnx-port-tests/test_ptpd    # its on-target test, from tests-8.0-<arch>-m-z
apk del ptpd ptpd-dev qnx-test-ptpd
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/ptpd.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/ptpd.list
rm var/lib/manual/ptpd.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ptpd` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b ptpd.2.3.1 --recurse-submodules --shallow-submodules \
    https://github.com/ptpd/ptpd.git src/ptpd
```

### 3. Apply the QNX patches

4 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/ptpd < ports/ptpd/patches/0001-qnx-platform.patch
patch -p1 -d src/ptpd < ports/ptpd/patches/0002-qnx-no-bintime.patch
patch -p1 -d src/ptpd < ports/ptpd/patches/0003-strchr-not-index.patch
patch -p1 -d src/ptpd < ports/ptpd/patches/0004-qnx-adjtime-clockadjust.patch
```

**[`0001-qnx-platform.patch`](patches/0001-qnx-platform.patch)** — Build on QNX, as one of the BSD-style systems.

> constants_dep.h stops with "#error PTPD has not been ported to this OS" on
> anything but Linux, the BSDs, macOS and Solaris. QNX 8 has the BSD network
> API (io-sock is FreeBSD-derived): <net/if_dl.h>, getifaddrs(), IF_NAMESIZE,
> struct ether_addr with "octet", so it takes the BSD branch of the platform
> constants.

Touches: `src/dep/constants_dep.h` · Upstream status: QNX-specific

**[`0002-qnx-no-bintime.patch`](patches/0002-qnx-no-bintime.patch)** — Use SO_TIMESTAMP, not SO_BINTIME, for receive timestamps on QNX.

> QNX's io-sock defines SO_BINTIME and SCM_BINTIME, but has neither
> struct bintime for applications (it is only in the driver headers) nor
> bintime2timespec() in libc, so the BINTIME path does not link. ptpd falls
> back to SO_TIMESTAMP (struct timeval), which QNX supports.

Touches: `src/dep/net.c` · Upstream status: QNX-specific

**[`0003-strchr-not-index.patch`](patches/0003-strchr-not-index.patch)** — Use strchr(), not the legacy index().

> index() was removed from POSIX in 2008 and QNX's libc does not have it
> (the link fails on an undefined reference). strchr() is the standard
> equivalent.

Touches: `src/dep/daemonconfig.c` · Upstream status: QNX-specific

**[`0004-qnx-adjtime-clockadjust.patch`](patches/0004-qnx-adjtime-clockadjust.patch)** — Slew the clock with ClockAdjust() on QNX.

> Without <sys/timex.h> ptpd corrects the clock with adjtime(), which QNX
> does not have. QNX's interface for slewing is ClockAdjust(): a per-tick
> increment applied for a number of ticks. adjTime() spreads the correction
> over about one second, at most 2% of the tick per tick (the limit ntp's
> QNX adjtime() emulation uses); a larger correction takes longer.

Touches: `src/dep/sys.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/ptpd && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/ptpd && cd build/$QNX_ARCH/ptpd
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
ac_cv_func_malloc_0_nonnull=yes \
ac_cv_header_net_ethernet_h=yes \
ac_cv_member_struct_ether_addr_octet=yes \
ac_cv_header_utmp_h=no \
$OLDPWD/src/ptpd/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-pcap \
    --disable-snmp \
    --enable-statistics
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- PTPd 2.x, the IEEE 1588 Precision Time Protocol daemon (the 2.3.1 release).
- The git tree has no configure: qnxbuild runs autoreconf. UDP transport only (IPv4 multicast, hybrid or unicast): no libpcap Ethernet transport, no net-snmp MIB.
- QNX's <net/ethernet.h> needs <sys/cdefs.h> (__packed) before it, which configure's bare header probe does not include: tell configure what is there (struct ether_addr with the BSD "octet" member). ptpd's own sources include it after <sys/socket.h>, which brings <sys/cdefs.h>. QNX's struct utmp has ut_type/OLD_TIME but no ut_tv: no utmp/wtmp records of clock steps (the only use ptpd has for utmp). malloc(0) returns a valid pointer on QNX (hazard 73).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ptpd`.

### 5. What it installs

7 files under `/usr/local`, 1 programs.

```
/usr/local/sbin/ptpd2
```

The complete list is `stage/<arch>/.manifest/ptpd` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ptpd
ls dists/8.0/*/ | grep '^ptpd-'
```

This writes `ptpd-<ver>-r0.tar.gz` (run time) and `ptpd-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ptpd
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ptpd-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ptpd-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_ptpd`](../../tests/src/) links ptpd the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-ptpd`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-ptpd
/usr/local/bin/qnx-port-tests/test_ptpd
```

```
== ptpd ==
  ok   ...
PASS ptpd
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_ptpd` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_ptpd`](../../tests/src/) -- a program linked against ptpd the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ptpd 2.3.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/ptpd-2.3.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ptpd 2.3.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/ptpd-2.3.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.3.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ptpd=<version>`), sources in `ported/ptpd/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ptpd`; port one with `scripts/port-update ptpd`.

<details><summary>recipe.sh</summary>

```bash
# PTPd 2.x, the IEEE 1588 Precision Time Protocol daemon (the 2.3.1 release).
VERSION=2.3.1; GIT=https://github.com/ptpd/ptpd.git; REF=ptpd.2.3.1
BUILDSYS=autotools
# The git tree has no configure: qnxbuild runs autoreconf.
# UDP transport only (IPv4 multicast, hybrid or unicast): no libpcap
# Ethernet transport, no net-snmp MIB.
CONF_ARGS_EXTRA=( --disable-pcap --disable-snmp --enable-statistics )
# QNX's <net/ethernet.h> needs <sys/cdefs.h> (__packed) before it, which
# configure's bare header probe does not include: tell configure what is
# there (struct ether_addr with the BSD "octet" member). ptpd's own sources
# include it after <sys/socket.h>, which brings <sys/cdefs.h>.
# QNX's struct utmp has ut_type/OLD_TIME but no ut_tv: no utmp/wtmp records
# of clock steps (the only use ptpd has for utmp).
# malloc(0) returns a valid pointer on QNX (hazard 73).
CONF_ENV=( ac_cv_func_malloc_0_nonnull=yes ac_cv_header_net_ethernet_h=yes ac_cv_member_struct_ether_addr_octet=yes ac_cv_header_utmp_h=no )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
