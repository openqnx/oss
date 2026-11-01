# inetutils 2.6

> A collection of common network programs (GNU inetutils)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/inetutils/inetutils-2.6.tar.xz |
| SHA-256 | `68bedbfeaf73f7d86be2a7d99bcfbd4093d829f52770893919ae174c0b2357ca` |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | [`ncurses`](../ncurses/) [`readline`](../readline/) [`libxcrypt`](../libxcrypt/) |
| Run-time dependencies | [`libxcrypt`](../libxcrypt/) [`ncurses`](../ncurses/) [`readline`](../readline/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_inetutils`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `inetutils` (programs, shared libraries, run-time data) and `inetutils-dev` (headers, pkg-config/CMake files, static libraries; needs `inetutils`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`inetutils`](https://openqnx.github.io/oss/package/x86_64/inetutils.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/inetutils.html)) · [`inetutils-dev`](https://openqnx.github.io/oss/package/x86_64/inetutils-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/inetutils-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add inetutils` (dependencies come along); one version: `apk add inetutils=<version>` | `apk del inetutils` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install inetutils` (dependencies first) | `oss-get remove inetutils` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`inetutils-dev` the same way, by its own name (`apk add inetutils-dev`, `oss-get install inetutils-dev`, by hand with its own file and list `inetutils-dev.list`); remove it before `inetutils`, which it needs.

```sh
# on the QNX target, as root
apk add inetutils inetutils-dev
apk add qnx-test-inetutils && /usr/local/bin/qnx-port-tests/test_inetutils    # its on-target test, from tests-8.0-<arch>
apk del inetutils inetutils-dev qnx-test-inetutils
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/inetutils.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/inetutils.list
rm var/lib/manual/inetutils.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild inetutils` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild ncurses readline libxcrypt
```

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/inetutils/inetutils-2.6.tar.xz
mkdir -p dist src/inetutils
curl -fL -o dist/$(basename $URL) $URL
echo "68bedbfeaf73f7d86be2a7d99bcfbd4093d829f52770893919ae174c0b2357ca  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/inetutils --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/inetutils < ports/inetutils/patches/0001-qnx-readutmp-sys-time.patch
patch -p1 -d src/inetutils < ports/inetutils/patches/0002-qnx-telnet-sa-restart.patch
```

**[`0001-qnx-readutmp-sys-time.patch`](patches/0001-qnx-readutmp-sys-time.patch)** — Include <sys/time.h> for the sysctl boot-time fallback.

> readutmp.c includes <sys/sysctl.h> for the KERN_BOOTTIME fallback of
> boot-time-aux.h, which fills a struct timeval. On the BSDs <sys/sysctl.h>
> brings <sys/time.h> along; QNX's does not, so struct timeval was
> incomplete ("storage size of result is not known").

Touches: `lib/readutmp.c` · Upstream status: QNX-specific

**[`0002-qnx-telnet-sa-restart.patch`](patches/0002-qnx-telnet-sa-restart.patch)** — telnet: do without SA_RESTART where the system lacks it.

> QNX's <signal.h> leaves SA_RESTART commented out ("not supported yet").
> telnet asks for restartable system calls only for its SIGWINCH/SIGINFO
> handlers; its main loop already treats EINTR from select() as "try again".

Touches: `telnet/sys_bsd.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/inetutils; B=build/$QNX_ARCH/inetutils; mkdir -p $B
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" SUIDMODE="-m 4755" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- crypt() for ftpd: QNX keeps it in its static liblogin.a, not libc, so libxcrypt (libcrypt.so.2) supplies it; without it configure drops ftpd.
- The subset of GNU inetutils that builds and works on QNX: telnet and telnetd, ftp and ftpd, ping/ping6, traceroute, whois, hostname, dnsdomainname, logger. Not built: tftp/tftpd (tftp-hpa is ported and owns those names), inetd, syslogd, rsh/rlogin/rexec/rcp and their daemons, talk, uucpd, ifconfig (whose interface code is Linux/BSD ioctl-specific). telnetd starts QNX's own /bin/login (inetutils would take $bindir/login, /usr/local/bin/login, which nothing installs). telnetd and ftpd are inetd-style servers (the connection on stdin).
- ping, ping6 and traceroute are installed setuid root with "install -o root -m 4755", which an unprivileged build cannot do: the chown fails and leaves the files mode 0600 (not executable). Install them without the owner flag; the mode, setuid included, is kept, and scripts/qnxpkg records every file as root.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH inetutils`.

### 5. What it installs

23 files under `/usr/local`, 9 programs.

```
/usr/local/bin/dnsdomainname
/usr/local/bin/ftp
/usr/local/bin/hostname
/usr/local/bin/logger
/usr/local/bin/ping
/usr/local/bin/ping6
/usr/local/bin/telnet
/usr/local/bin/traceroute
/usr/local/bin/whois
```

The complete list is `stage/<arch>/.manifest/inetutils` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa inetutils
ls dists/8.0/*/ | grep '^inetutils-'
```

This writes `inetutils-<ver>-r0.tar.gz` (run time) and `inetutils-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add inetutils
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/inetutils-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/inetutils-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_inetutils`](../../tests/src/) links inetutils the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-inetutils`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-inetutils
/usr/local/bin/qnx-port-tests/test_inetutils
```

```
== inetutils ==
  ok   ...
PASS inetutils
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_inetutils` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_inetutils`](../../tests/src/) -- a program linked against inetutils the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | inetutils 2.6-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/inetutils-2.6-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | inetutils 2.6-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/inetutils-2.6-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add inetutils=<version>`), sources in `ported/inetutils/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates inetutils`; port one with `scripts/port-update inetutils`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.6
TARBALL=https://ftp.gnu.org/gnu/inetutils/inetutils-2.6.tar.xz
SHA256=68bedbfeaf73f7d86be2a7d99bcfbd4093d829f52770893919ae174c0b2357ca
BUILDSYS=autotools
# crypt() for ftpd: QNX keeps it in its static liblogin.a, not libc, so
# libxcrypt (libcrypt.so.2) supplies it; without it configure drops ftpd.
DEPS="ncurses readline libxcrypt"
# The subset of GNU inetutils that builds and works on QNX: telnet and
# telnetd, ftp and ftpd, ping/ping6, traceroute, whois, hostname,
# dnsdomainname, logger. Not built: tftp/tftpd (tftp-hpa is ported and owns
# those names), inetd, syslogd, rsh/rlogin/rexec/rcp and their daemons, talk,
# uucpd, ifconfig (whose interface code is Linux/BSD ioctl-specific).
# telnetd starts QNX's own /bin/login (inetutils would take $bindir/login,
# /usr/local/bin/login, which nothing installs).
# telnetd and ftpd are inetd-style servers (the connection on stdin).
CONF_ARGS_EXTRA=( --disable-servers --disable-clients
                  --enable-telnet --enable-telnetd --enable-ftp --enable-ftpd
                  --enable-ping --enable-ping6 --enable-traceroute --enable-whois
                  --enable-hostname --enable-dnsdomainname --enable-logger
                  --disable-nls --without-pam --without-krb5 --without-shishi
                  --with-path-login=/bin/login )
# ping, ping6 and traceroute are installed setuid root with "install -o
# root -m 4755", which an unprivileged build cannot do: the chown fails and
# leaves the files mode 0600 (not executable). Install them without the
# owner flag; the mode, setuid included, is kept, and scripts/qnxpkg records
# every file as root.
do_install() { make -C "$B" DESTDIR="$STAGE" SUIDMODE="-m 4755" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
