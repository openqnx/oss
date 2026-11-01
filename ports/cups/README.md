# cups 2.4.16

> Standards-based printing system: scheduler, libcups and command-line tools

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/OpenPrinting/cups.git (tag `v2.4.16`) |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) [`avahi`](../avahi/) [`libiconv`](../libiconv/) [`libxcrypt`](../libxcrypt/) |
| Run-time dependencies | [`avahi`](../avahi/) [`libiconv`](../libiconv/) [`libxcrypt`](../libxcrypt/) [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_cups`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `cups` (programs, shared libraries, run-time data) and `cups-dev` (headers, pkg-config/CMake files, static libraries; needs `cups`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`cups`](https://openqnx.github.io/oss/package/x86_64/cups.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/cups.html)) · [`cups-dev`](https://openqnx.github.io/oss/package/x86_64/cups-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/cups-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add cups` (dependencies come along); one version: `apk add cups=<version>` | `apk del cups` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install cups` (dependencies first) | `oss-get remove cups` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`cups-dev` the same way, by its own name (`apk add cups-dev`, `oss-get install cups-dev`, by hand with its own file and list `cups-dev.list`); remove it before `cups`, which it needs.

```sh
# on the QNX target, as root
apk add cups cups-dev
apk add qnx-test-cups && /usr/local/bin/qnx-port-tests/test_cups    # its on-target test, from tests-8.0-<arch>
apk del cups cups-dev qnx-test-cups
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/cups.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/cups.list
rm var/lib/manual/cups.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild cups` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (8 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild openssl zlib avahi libiconv libxcrypt
```

<details><summary>Full build chain (8)</summary>

[`avahi`](../avahi/) [`dbus`](../dbus/) [`expat`](../expat/) [`libdaemon`](../libdaemon/) [`libiconv`](../libiconv/) [`libxcrypt`](../libxcrypt/) [`openssl`](../openssl/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v2.4.16 --recurse-submodules --shallow-submodules \
    https://github.com/OpenPrinting/cups.git src/cups
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/cups < ports/cups/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/cups < ports/cups/patches/0002-qnx-pass-ld-library-path.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — qnx support

> Imported from qnx-ports/cups (branch qnx-2.4.x, 7523763f..f5baad1c); original authorship kept.
>
> ---
>  config-scripts/cups-sharedlibs.m4 | 2 +-
>  cups/http-addrlist.c              | 3 +++
>  scheduler/auth.c                  | 3 +++
>  3 files changed, 7 insertions(+), 1 deletion(-)

Touches: `config-scripts/cups-sharedlibs.m4`, `cups/http-addrlist.c`, `scheduler/auth.c` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-pass-ld-library-path.patch`](patches/0002-qnx-pass-ld-library-path.patch)** — scheduler: pass LD_LIBRARY_PATH to filters and backends on QNX

> cupsd, running as root, gives its filters, backends and helper programs a
> clean environment and refuses to pass on any LD_ variable. On QNX the
> libraries of anything installed outside the system directories (libcups
> itself under /usr/local/lib, and on a minimal image QNX's own
> libeventfd/libsocket companions in a separate directory) are only found
> through LD_LIBRARY_PATH, so no filter could start ("Could not load library
> libcups.so.2"). Pass LD_LIBRARY_PATH -- the one cupsd itself was started
> with -- on QNX; LD_PRELOAD and the other loader variables stay blocked.

Touches: `scheduler/env.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/cups; B=build/$QNX_ARCH/cups; mkdir -p $B
```

```bash
do_patch() { ( cd "$SRCROOT" && autoconf -f ); }
```

```bash
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
  CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
  CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
  CFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" CXXFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
  LDFLAGS="-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state -L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -Wl,-z,stack-size=8388608 -Wl,-z,noexecstack -Wl,-rpath,$PREFIX/lib -Wl,--disable-new-dtags" \
  LIBS="-lsocket -lregex" \
  ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" --sysconfdir="$PREFIX/etc" --localstatedir="$PREFIX/var" \
    --with-tls=openssl --with-dnssd=avahi --disable-dbus --disable-pam --disable-gssapi --disable-libusb \
    --disable-libpaper --disable-acl --with-ondemand=no --without-systemd --without-rcdir \
    --with-languages=none --disable-static --enable-shared \
    --with-icondir="$PREFIX/share/icons" --with-menudir="$PREFIX/share/applications"
}
```

```bash
do_build() { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" BUILDROOT="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- CUPS, the printing system: libcups and libcupsimage, the scheduler (cupsd), the command-line tools (lp, lpr, lpstat, lpadmin, cancel, cupsctl, ipptool, ippfind, ippeveprinter ...), and the backends and filters. 2.4.16 with qnx-ports' QNX support (patches/0001: s6_addr32 is spelled __u6_addr.__u6_addr32 in QNX's <netinet6/in6.h>, and programs link libcups together with its own libraries); patches/0002 lets cupsd pass LD_LIBRARY_PATH on to its filters and backends (see below).
- libxcrypt: crypt() for cupsd's Basic authentication against local accounts (configure links -lcrypt, which QNX's libc does not provide).
- patches/0001 changes config-scripts/cups-sharedlibs.m4 (programs link libcups together with its own libraries: QNX's GNU ld does not resolve a program's direct references, regexec from libregex say, through libcups's DT_NEEDED); the shipped configure is regenerated from it.
- CUPS's makefiles build in the source tree only (Makedefs at the top), so configure and build in a copy of it, with the same compiler settings the autotools driver uses. TLS: OpenSSL. DNS-SD: Avahi (printer sharing and ippfind). Off: PAM and Kerberos (neither exists on QNX), libusb (no USB backend: QNX's USB stack is not libusb's), systemd/launchd on-demand start, D-Bus notifications, the translations of the web interface (--with-languages=none). -z stack-size: ipptool's do_test() keeps a 1 MiB copy buffer on the stack, far more than QNX gives the main thread by default (hazard 119): it died with SIGSEGV on its first request. 8 MiB, Linux's default, for every program here (with -z noexecstack: the stack-size header would otherwise come out executable). -rpath $PREFIX/lib (DT_RPATH, which covers the libraries' own dependencies too): cupsd runs its filters, backends and helpers with a clean environment and, as root, refuses to pass any LD_* variable on (scheduler/env.c). /usr/local/lib is not on QNX's default library path, so every filter failed to start ("Could not load library libcups.so.2"). The loader still takes LD_LIBRARY_PATH first where it is set -- and with patches/0002 cupsd passes its own LD_LIBRARY_PATH on, which a minimal image needs for QNX's libeventfd & co. in a directory of their own. -lregex: regcomp()/regexec() (ipptool, the scheduler) are in QNX's libregex, not libc. State (spool, cache, logs, run) under $PREFIX/var; the icon and desktop file under the prefix too (CUPS's defaults are /usr/share/...).
- BUILDROOT is CUPS's DESTDIR.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH cups`.

### 5. What it installs

383 files under `/usr/local`: 4 shared-library entries, 28 programs, 15 headers.

```
/usr/local/lib/libcups.so
/usr/local/lib/libcups.so.2
/usr/local/lib/libcupsimage.so
/usr/local/lib/libcupsimage.so.2
/usr/local/bin/cancel
/usr/local/bin/cups-config
/usr/local/bin/cupstestppd
/usr/local/bin/ippeveprinter
/usr/local/bin/ippfind
/usr/local/bin/ipptool
/usr/local/bin/lp
/usr/local/bin/lpoptions
/usr/local/bin/lpq
/usr/local/bin/lpr
/usr/local/bin/lprm
/usr/local/bin/lpstat
...
```

The complete list is `stage/<arch>/.manifest/cups` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa cups
ls dists/8.0/*/ | grep '^cups-'
```

This writes `cups-<ver>-r0.tar.gz` (run time) and `cups-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add cups
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/cups-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/cups-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_cups`](../../tests/src/) links cups the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-cups`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-cups
/usr/local/bin/qnx-port-tests/test_cups
```

```
== cups ==
  ok   ...
PASS cups
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_cups` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_cups`](../../tests/src/) -- a program linked against cups the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | cups 2.4.16-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/cups-2.4.16-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | cups 2.4.16-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/cups-2.4.16-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.4.16**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add cups=<version>`), sources in `ported/cups/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates cups`; port one with `scripts/port-update cups`.

<details><summary>recipe.sh</summary>

```bash
# CUPS, the printing system: libcups and libcupsimage, the scheduler (cupsd),
# the command-line tools (lp, lpr, lpstat, lpadmin, cancel, cupsctl, ipptool,
# ippfind, ippeveprinter ...), and the backends and filters. 2.4.16 with
# qnx-ports' QNX support (patches/0001: s6_addr32 is spelled
# __u6_addr.__u6_addr32 in QNX's <netinet6/in6.h>, and programs link libcups
# together with its own libraries); patches/0002 lets cupsd pass
# LD_LIBRARY_PATH on to its filters and backends (see below).
VERSION=2.4.16; GIT=https://github.com/OpenPrinting/cups.git; REF=v2.4.16
# libxcrypt: crypt() for cupsd's Basic authentication against local accounts
# (configure links -lcrypt, which QNX's libc does not provide).
DEPS="openssl zlib avahi libiconv libxcrypt"
BUILDSYS=custom
# patches/0001 changes config-scripts/cups-sharedlibs.m4 (programs link
# libcups together with its own libraries: QNX's GNU ld does not resolve a
# program's direct references, regexec from libregex say, through libcups's
# DT_NEEDED); the shipped configure is regenerated from it.
do_patch() { ( cd "$SRCROOT" && autoconf -f ); }
# CUPS's makefiles build in the source tree only (Makedefs at the top), so
# configure and build in a copy of it, with the same compiler settings the
# autotools driver uses.
# TLS: OpenSSL. DNS-SD: Avahi (printer sharing and ippfind). Off: PAM and
# Kerberos (neither exists on QNX), libusb (no USB backend: QNX's USB stack is
# not libusb's), systemd/launchd on-demand start, D-Bus notifications, the
# translations of the web interface (--with-languages=none).
# -z stack-size: ipptool's do_test() keeps a 1 MiB copy buffer on the stack,
# far more than QNX gives the main thread by default (hazard 119): it died
# with SIGSEGV on its first request. 8 MiB, Linux's default, for every
# program here (with -z noexecstack: the stack-size header would otherwise
# come out executable).
# -rpath $PREFIX/lib (DT_RPATH, which covers the libraries' own
# dependencies too): cupsd runs its filters, backends and helpers with a
# clean environment and, as root, refuses to pass any LD_* variable on
# (scheduler/env.c). /usr/local/lib is not on QNX's default library path, so
# every filter failed to start ("Could not load library libcups.so.2"). The
# loader still takes LD_LIBRARY_PATH first where it is set -- and with
# patches/0002 cupsd passes its own LD_LIBRARY_PATH on, which a minimal
# image needs for QNX's libeventfd & co. in a directory of their own.
# -lregex: regcomp()/regexec() (ipptool, the scheduler) are in QNX's libregex,
# not libc.
# State (spool, cache, logs, run) under $PREFIX/var; the icon and desktop
# file under the prefix too (CUPS's defaults are /usr/share/...).
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
  CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
  CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
  CFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" CXXFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
  LDFLAGS="-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state -L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -Wl,-z,stack-size=8388608 -Wl,-z,noexecstack -Wl,-rpath,$PREFIX/lib -Wl,--disable-new-dtags" \
  LIBS="-lsocket -lregex" \
  ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" --sysconfdir="$PREFIX/etc" --localstatedir="$PREFIX/var" \
    --with-tls=openssl --with-dnssd=avahi --disable-dbus --disable-pam --disable-gssapi --disable-libusb \
    --disable-libpaper --disable-acl --with-ondemand=no --without-systemd --without-rcdir \
    --with-languages=none --disable-static --enable-shared \
    --with-icondir="$PREFIX/share/icons" --with-menudir="$PREFIX/share/applications"
}
do_build() { make -C "$B" -j"$(nproc)"; }
# BUILDROOT is CUPS's DESTDIR.
do_install() { make -C "$B" BUILDROOT="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
