# perl-io-tty 1.31

> Perl IO::Tty and IO::Pty: pseudo-terminals and terminal constants

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://cpan.metacpan.org/authors/id/T/TO/TODDR/IO-Tty-1.31.tar.gz |
| SHA-256 | `d597af221628571cbecf35b44520148c44798dfc8a9867774e60453f79d25ff7` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`perl`](../perl/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_perl_io_tty`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `perl-io-tty` (programs, shared libraries, run-time data) and `perl-io-tty-dev` (headers, pkg-config/CMake files, static libraries; needs `perl-io-tty`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`perl-io-tty`](https://openqnx.github.io/oss/package/x86_64/perl-io-tty.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/perl-io-tty.html)) · [`perl-io-tty-dev`](https://openqnx.github.io/oss/package/x86_64/perl-io-tty-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/perl-io-tty-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add perl-io-tty` (dependencies come along); one version: `apk add perl-io-tty=<version>` | `apk del perl-io-tty` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install perl-io-tty` (dependencies first) | `oss-get remove perl-io-tty` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`perl-io-tty-dev` the same way, by its own name (`apk add perl-io-tty-dev`, `oss-get install perl-io-tty-dev`, by hand with its own file and list `perl-io-tty-dev.list`); remove it before `perl-io-tty`, which it needs.

```sh
# on the QNX target, as root
apk add perl-io-tty perl-io-tty-dev
apk add qnx-test-perl-io-tty && /usr/local/bin/qnx-port-tests/test_perl_io_tty    # its on-target test, from tests-8.0-<arch>-m-z
apk del perl-io-tty perl-io-tty-dev qnx-test-perl-io-tty
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/perl-io-tty.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/perl-io-tty.list
rm var/lib/manual/perl-io-tty.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild perl-io-tty` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild perl
```

<details><summary>Full build chain (2)</summary>

[`libxcrypt`](../libxcrypt/) [`perl`](../perl/)
</details>

### 2. Get the upstream source

```bash
URL=https://cpan.metacpan.org/authors/id/T/TO/TODDR/IO-Tty-1.31.tar.gz
mkdir -p dist src/perl-io-tty
curl -fL -o dist/$(basename $URL) $URL
echo "d597af221628571cbecf35b44520148c44798dfc8a9867774e60453f79d25ff7  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/perl-io-tty --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/perl-io-tty < ports/perl-io-tty/patches/0001-probe-sys-ioctl-h.patch
```

**[`0001-probe-sys-ioctl-h.patch`](patches/0001-probe-sys-ioctl-h.patch)** — Makefile.PL: look for the terminal ioctls in <sys/ioctl.h> too

> Makefile.PL decides which terminal constants exist by compiling a test
> for each with the headers it found, and those are only termios.h,
> termio.h, pty.h and friends. On Linux <termios.h> pulls in the ioctl
> numbers; on QNX (and the BSDs) TIOCSWINSZ, TIOCGWINSZ, TIOCSCTTY,
> TIOCNOTTY ... are in <sys/ioctl.h>, so all of them came out undefined:
> IO::Pty could not set the window size and Pty.pm warned that TIOCSCTTY
> did not compile. Tty.xs already includes <sys/ioctl.h>; let the probes see
> it too.

Touches: `Makefile.PL` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/perl-io-tty; B=build/$QNX_ARCH/perl-io-tty; mkdir -p $B
```

```bash
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && \
  PERL_MM_USE_DEFAULT=1 perl Makefile.PL CC="qcc -V$QCC_TARGET" CCFLAGS="$(_pcfg ccflags)" OPTIMIZE="-O2" LDFLAGS="" \
    INSTALLDIRS=site && \
  sed -i 's/-DHAVE_DEV_PTMX //g' Makefile
}
```

```bash
do_build() { _pmk; }
```

```bash
do_install() {
  _pmk DESTDIR="$STAGE" install && \
  find "$STAGE$PREFIX/lib/perl5" \( -name perllocal.pod -o -name .packlist \) -delete
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- IO::Tty and IO::Pty (pseudo-terminals for Perl; what Expect and IPC::Run's pty support are built on), an XS module, built for the target perl of this tree. patches/0001: the terminal ioctls (TIOCSWINSZ, TIOCSCTTY ...) are probed in <sys/ioctl.h>, where QNX declares them.
- Cross build: Makefile.PL only compiles and links its probes (openpty, ptsname, posix_openpt, the termios constants...), never runs them, so the build machine's perl runs it with the QNX compiler named on its command line; it writes xssubs.c, IO/Tty/Constant.pm and the -DHAVE_... list. The generated Makefile is then driven with the target perl's settings, read from the staged Config_heavy.pl: its CORE headers, ccflags, -fPIC and -shared, and its site arch directory (site_perl/<version>/<archname>, on the target's @INC). xsubpp is the build machine's ExtUtils::ParseXS; its output is plain C compiled against the target perl's headers. PERL_HDRS is emptied: it lists the build machine perl's headers (patchlevel-debian.h ...) as dependencies of Tty.o. Makefile.PL also looks for clone devices (/dev/ptmx ...) on the machine it runs on; that answer is the build machine's, so -DHAVE_DEV_PTMX is taken out again. IO::Tty opens ptys with posix_openpt(), which QNX has and which the module tries first.
- perllocal.pod (every module install appends to it: a file two packages would share) and .packlist (it lists the DESTDIR paths) are not shipped.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH perl-io-tty`.

### 5. What it installs

7 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/perl-io-tty` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa perl-io-tty
ls dists/8.0/*/ | grep '^perl-io-tty-'
```

This writes `perl-io-tty-<ver>-r0.tar.gz` (run time) and `perl-io-tty-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add perl-io-tty
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/perl-io-tty-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/perl-io-tty-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_perl_io_tty`](../../tests/src/) links perl-io-tty the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-perl-io-tty`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-perl-io-tty
/usr/local/bin/qnx-port-tests/test_perl_io_tty
```

```
== perl-io-tty ==
  ok   ...
PASS perl-io-tty
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_perl_io_tty` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_perl_io_tty`](../../tests/src/) -- a program linked against perl-io-tty the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | perl-io-tty 1.31-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/perl-io-tty-1.31-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | perl-io-tty 1.31-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/perl-io-tty-1.31-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.31**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add perl-io-tty=<version>`), sources in `ported/perl-io-tty/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates perl-io-tty`; port one with `scripts/port-update perl-io-tty`.

<details><summary>recipe.sh</summary>

```bash
# IO::Tty and IO::Pty (pseudo-terminals for Perl; what Expect and
# IPC::Run's pty support are built on), an XS module, built for the target
# perl of this tree. patches/0001: the terminal ioctls (TIOCSWINSZ,
# TIOCSCTTY ...) are probed in <sys/ioctl.h>, where QNX declares them.
VERSION=1.31
TARBALL=https://cpan.metacpan.org/authors/id/T/TO/TODDR/IO-Tty-1.31.tar.gz
SHA256=d597af221628571cbecf35b44520148c44798dfc8a9867774e60453f79d25ff7
DEPS="perl"
BUILDSYS=custom
# Cross build: Makefile.PL only compiles and links its probes (openpty,
# ptsname, posix_openpt, the termios constants...), never runs them, so the
# build machine's perl runs it with the QNX compiler named on its command
# line; it writes xssubs.c, IO/Tty/Constant.pm and the -DHAVE_... list. The
# generated Makefile is then driven with the target perl's settings, read
# from the staged Config_heavy.pl: its CORE headers, ccflags, -fPIC and
# -shared, and its site arch directory (site_perl/<version>/<archname>, on
# the target's @INC). xsubpp is the build machine's ExtUtils::ParseXS; its
# output is plain C compiled against the target perl's headers.
# PERL_HDRS is emptied: it lists the build machine perl's headers
# (patchlevel-debian.h ...) as dependencies of Tty.o.
# Makefile.PL also looks for clone devices (/dev/ptmx ...) on the machine it
# runs on; that answer is the build machine's, so -DHAVE_DEV_PTMX is taken
# out again. IO::Tty opens ptys with posix_openpt(), which QNX has and which
# the module tries first.
_pcfg() { sed -n "s/^$1='\(.*\)'\$/\1/p" "$(ls "$STAGE_SYSROOT$PREFIX"/lib/perl5/*/*/Config_heavy.pl | head -n 1)"; }
_pmk() {
  local core; core=$(dirname "$(ls "$STAGE_SYSROOT$PREFIX"/lib/perl5/*/*/Config_heavy.pl | head -n 1)")/CORE
  make -C "$B" CC="qcc -V$QCC_TARGET" LD="qcc -V$QCC_TARGET" CCFLAGS="$(_pcfg ccflags)" OPTIMIZE="-O2" \
    CCCDLFLAGS="$(_pcfg cccdlflags)" LDDLFLAGS="$(_pcfg lddlflags)" DLEXT=so LDFLAGS= \
    PERL_INC="$core" PERL_ARCHLIBDEP="$core/.." PERL_INCDEP="$core" PERL_HDRS= \
    INSTALLSITEARCH="$(_pcfg installsitearch)" INSTALLSITELIB="$(_pcfg installsitearch)" \
    INSTALLSITEMAN3DIR="$PREFIX/share/man/man3" "$@"
}
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && \
  PERL_MM_USE_DEFAULT=1 perl Makefile.PL CC="qcc -V$QCC_TARGET" CCFLAGS="$(_pcfg ccflags)" OPTIMIZE="-O2" LDFLAGS="" \
    INSTALLDIRS=site && \
  sed -i 's/-DHAVE_DEV_PTMX //g' Makefile
}
do_build() { _pmk; }
# perllocal.pod (every module install appends to it: a file two packages
# would share) and .packlist (it lists the DESTDIR paths) are not shipped.
do_install() {
  _pmk DESTDIR="$STAGE" install && \
  find "$STAGE$PREFIX/lib/perl5" \( -name perllocal.pod -o -name .packlist \) -delete
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
