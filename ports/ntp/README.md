# ntp 4.2.8p18

> Network Time Protocol reference implementation: ntpd, ntpq, ntpdate

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://downloads.nwtime.org/ntp/4.2.8/ntp-4.2.8p18.tar.gz |
| SHA-256 | `cf84c5f3fb1a295284942624d823fffa634144e096cfc4f9969ac98ef5f468e5` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`libevent`](../libevent/) [`libedit`](../libedit/) |
| Run-time dependencies | [`libedit`](../libedit/) [`libevent`](../libevent/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_ntp`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `ntp`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`ntp`](https://openqnx.github.io/oss/package/x86_64/ntp.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ntp.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add ntp` (dependencies come along); one version: `apk add ntp=<version>` | `apk del ntp` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install ntp` (dependencies first) | `oss-get remove ntp` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-ntp && /usr/local/bin/qnx-port-tests/test_ntp    # its on-target test, from tests-8.0-<arch>-m-z
apk del ntp qnx-test-ntp
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/ntp.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/ntp.list
rm var/lib/manual/ntp.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ntp` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (4 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libevent libedit
```

<details><summary>Full build chain (4)</summary>

[`libedit`](../libedit/) [`libevent`](../libevent/) [`ncurses`](../ncurses/) [`openssl`](../openssl/)
</details>

### 2. Get the upstream source

```bash
URL=https://downloads.nwtime.org/ntp/4.2.8/ntp-4.2.8p18.tar.gz
mkdir -p dist src/ntp
curl -fL -o dist/$(basename $URL) $URL
echo "cf84c5f3fb1a295284942624d823fffa634144e096cfc4f9969ac98ef5f468e5  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/ntp --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/ntp < ports/ntp/patches/0001-qnx-no-bintime.patch
```

**[`0001-qnx-no-bintime.patch`](patches/0001-qnx-no-bintime.patch)** — Use SO_TIMESTAMP, not SO_BINTIME, for packet timestamps on QNX.

> QNX's io-sock defines SO_BINTIME and SCM_BINTIME (it is FreeBSD-derived),
> but declares struct bintime only in its driver headers (<devs/sys/time.h>),
> not for applications, so the BINTIME receive-timestamp path does not
> compile. Take the next one ntp knows: SO_TIMESTAMP (struct timeval), which
> QNX supports.

Touches: `ntpd/ntp_io.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/ntp; B=build/$QNX_ARCH/ntp; mkdir -p $B
```

```bash
do_build() { _ntpmake -j6; }
```

```bash
do_install() { _ntpmake DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The reference NTP implementation (ntpd, ntpq, ntpdc, ntpdate, ...) from ntp.org / Network Time Foundation.
- ntp-keygen and ntpd use libevent 2 (found with pkg-config), ntpq/ntpdc libedit for their command line.
- ntp-wait, ntptrace and update-leap are Perl scripts (#!/usr/local/bin/perl); no DT_NEEDED shows that.
- NTPsec (a fork of this code) is ported too and installs ntpd, ntpq, ntptrace and ntpwait under the same names. The reference implementation goes in a prefix of its own, /usr/local/opt/ntp (bin/ sbin/ share/man/), so both can be installed side by side.
- Without OpenSSL: symmetric keys still work (ntp's own MD5); Autokey and SHA/AES-CMAC keys need it. Without sntp, which needs the bundled libevent (ntpdate covers the one-shot query). No signalled I/O: configure enables SIGIO-driven input on QNX, and ntpd then exits at startup with "ioctl(SIOCSPGRP) fails: Operation not supported" (io-sock does not take SIOCSPGRP); ntpd's select() loop serves the sockets instead.
- ntp-wait, ntptrace, update-leap and calc_tickadj are Perl scripts: their #! line names the target's perl (the perl port, /usr/local/bin/perl), not the build machine's /usr/bin/perl. Perl is needed only for those four. Configure would also take whatever else staging holds: libmd for MD5 (ntp has its own), Avahi's libdns_sd for mDNS registration, gettext's libintl, and a libelf for nlist() (only tickadj, not built here, uses it). None of them is needed: they are answered "no", so the binaries depend on exactly what DEPS says.
- tickadj (and calc_tickadj) read and patch the kernel's tick variables through /dev/kmem and nlist(): there is no such thing on QNX (the tick is ClockPeriod()'s), so they are neither built nor installed.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ntp`.

### 5. What it installs

264 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/ntp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ntp
ls dists/8.0/*/ | grep '^ntp-'
```

This writes `ntp-<ver>-r0.tar.gz` (run time) and `ntp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ntp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ntp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ntp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_ntp`](../../tests/src/) links ntp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-ntp`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-ntp
/usr/local/bin/qnx-port-tests/test_ntp
```

```
== ntp ==
  ok   ...
PASS ntp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_ntp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_ntp`](../../tests/src/) -- a program linked against ntp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ntp 4.2.8_p18-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/ntp-4.2.8_p18-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ntp 4.2.8_p18-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/ntp-4.2.8_p18-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.2.8p18**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ntp=<version>`), sources in `ported/ntp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ntp`; port one with `scripts/port-update ntp`.

<details><summary>recipe.sh</summary>

```bash
# The reference NTP implementation (ntpd, ntpq, ntpdc, ntpdate, ...) from
# ntp.org / Network Time Foundation.
VERSION=4.2.8p18
TARBALL=https://downloads.nwtime.org/ntp/4.2.8/ntp-4.2.8p18.tar.gz
SHA256=cf84c5f3fb1a295284942624d823fffa634144e096cfc4f9969ac98ef5f468e5
BUILDSYS=autotools
# ntp-keygen and ntpd use libevent 2 (found with pkg-config), ntpq/ntpdc
# libedit for their command line.
DEPS="libevent libedit"
# ntp-wait, ntptrace and update-leap are Perl scripts (#!/usr/local/bin/perl);
# no DT_NEEDED shows that.
RUNTIME_DEPS="perl"
# NTPsec (a fork of this code) is ported too and installs ntpd, ntpq,
# ntptrace and ntpwait under the same names. The reference implementation
# goes in a prefix of its own, /usr/local/opt/ntp (bin/ sbin/ share/man/),
# so both can be installed side by side.
PREFIX_NTP="$PREFIX/opt/ntp"
# Without OpenSSL: symmetric keys still work (ntp's own MD5); Autokey and
# SHA/AES-CMAC keys need it. Without sntp, which needs the bundled libevent
# (ntpdate covers the one-shot query).
# No signalled I/O: configure enables SIGIO-driven input on QNX, and ntpd
# then exits at startup with "ioctl(SIOCSPGRP) fails: Operation not
# supported" (io-sock does not take SIOCSPGRP); ntpd's select() loop
# serves the sockets instead.
CONF_ARGS_EXTRA=( --prefix="$PREFIX_NTP" --disable-signalled-io --without-crypto --without-sntp --disable-linuxcaps
                  --without-ntpsnmpd --with-yielding-select=yes --with-lineeditlibs=edit )
# ntp-wait, ntptrace, update-leap and calc_tickadj are Perl scripts: their
# #! line names the target's perl (the perl port, /usr/local/bin/perl), not
# the build machine's /usr/bin/perl. Perl is needed only for those four.
# Configure would also take whatever else staging holds: libmd for MD5 (ntp
# has its own), Avahi's libdns_sd for mDNS registration, gettext's libintl,
# and a libelf for nlist() (only tickadj, not built here, uses it). None of
# them is needed: they are answered "no", so the binaries depend on exactly
# what DEPS says.
CONF_ENV=( ac_cv_path_PATH_PERL=/usr/local/bin/perl ac_cv_search_MD5Init=no
           ac_cv_header_dns_sd_h=no ac_cv_search_DNSServiceRegister=no
           ac_cv_lib_intl_gettext=no ac_cv_search_nlist=no ac_cv_header_nlist_h=no )
# tickadj (and calc_tickadj) read and patch the kernel's tick variables
# through /dev/kmem and nlist(): there is no such thing on QNX (the tick
# is ClockPeriod()'s), so they are neither built nor installed.
_ntpmake() { make -C "$B" TICKADJ_DB= TICKADJ_MS= CALC_TICKADJ_DB= CALC_TICKADJ_MS= "$@"; }
do_build() { _ntpmake -j6; }
do_install() { _ntpmake DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
