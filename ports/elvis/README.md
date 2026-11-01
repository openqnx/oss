# elvis 2.2.1-git-20240226

> Clone of the ex/vi text editor

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/mbert/elvis.git |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`ncurses`](../ncurses/) |
| Run-time dependencies | [`ncurses`](../ncurses/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_elvis`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `elvis` (programs, shared libraries, run-time data) and `elvis-dev` (headers, pkg-config/CMake files, static libraries; needs `elvis`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`elvis`](https://openqnx.github.io/oss/package/x86_64/elvis.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/elvis.html)) · [`elvis-dev`](https://openqnx.github.io/oss/package/x86_64/elvis-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/elvis-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add elvis` (dependencies come along); one version: `apk add elvis=<version>` | `apk del elvis` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install elvis` (dependencies first) | `oss-get remove elvis` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`elvis-dev` the same way, by its own name (`apk add elvis-dev`, `oss-get install elvis-dev`, by hand with its own file and list `elvis-dev.list`); remove it before `elvis`, which it needs.

```sh
# on the QNX target, as root
apk add elvis elvis-dev
apk add qnx-test-elvis && /usr/local/bin/qnx-port-tests/test_elvis    # its on-target test, from tests-8.0-<arch>
apk del elvis elvis-dev qnx-test-elvis
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/elvis.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/elvis.list
rm var/lib/manual/elvis.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild elvis` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/mbert/elvis.git src/elvis
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/elvis; B=build/$QNX_ARCH/elvis; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/" && cd "$B" && sh ./configure --with-x=no --without-gcc --prefix="$PREFIX" qnx6 && \
  sed -i "s!^CC=.*!CC=qcc -V$QCC_TARGET -D_QNX_SOURCE -I$STAGE$PREFIX/include/ncursesw -I$STAGE$PREFIX/include!; \
          s!^LIBS=.*!LIBS=-L$STAGE$PREFIX/lib -lncursesw -lsocket!" Makefile; }
```

```bash
do_build() { make -C "$B" -j6; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install && rm -rf "$STAGE/etc/elvis" && rmdir "$STAGE/etc" 2>/dev/null; :; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- elvis, the vi clone, from its maintained repository (the 2.2_1 line: 65 commits of fixes after the last tag, v2.2_1-pre3).
- elvis's configure is hand-written: given the system name "qnx6" (QNX Neutrino, which 8.0 still is) it writes config.h and the Makefile without running anything. Its header probes read the build machine's /usr/include, which agrees with QNX here (termios, select, sockets, setpgid). The termcap interface uses ncurses' termcap API (tgetent and friends in libncursesw, <termcap.h> in include/ncursesw), not the -ltermcap of QNX 6.
- The install also writes $DESTDIR/etc/elvis/README, a note for an administrator's /etc/elvis overrides; a package must not own files in the target's /etc, so it goes (elvis still reads /etc/elvis when it exists). It runs the just-built elvtags to index data/stubs: on the build machine that falls back to the host's ctags, or leaves no tags file, as upstream allows ("-" in the Makefile).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH elvis`.

### 5. What it installs

101 files under `/usr/local`, 4 programs.

```
/usr/local/bin/elvfmt
/usr/local/bin/elvis
/usr/local/bin/elvtags
/usr/local/bin/ref
```

The complete list is `stage/<arch>/.manifest/elvis` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa elvis
ls dists/8.0/*/ | grep '^elvis-'
```

This writes `elvis-<ver>-r0.tar.gz` (run time) and `elvis-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add elvis
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/elvis-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/elvis-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_elvis`](../../tests/src/) links elvis the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-elvis`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-elvis
/usr/local/bin/qnx-port-tests/test_elvis
```

```
== elvis ==
  ok   ...
PASS elvis
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_elvis` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_elvis`](../../tests/src/) -- a program linked against elvis the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | elvis 2.2.1_git20240226-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/elvis-2.2.1_git20240226-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | elvis 2.2.1_git20240226-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/elvis-2.2.1_git20240226-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.2.1-git-20240226**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add elvis=<version>`), sources in `ported/elvis/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates elvis`; port one with `scripts/port-update elvis`.

<details><summary>recipe.sh</summary>

```bash
# elvis, the vi clone, from its maintained repository (the 2.2_1 line: 65
# commits of fixes after the last tag, v2.2_1-pre3).
VERSION=2.2.1-git-20240226; GIT=https://github.com/mbert/elvis.git; COMMIT=3684829e8596b9761c36629f20ff9d3262c10481
DEPS="ncurses"
BUILDSYS=custom
# elvis's configure is hand-written: given the system name "qnx6" (QNX
# Neutrino, which 8.0 still is) it writes config.h and the Makefile without
# running anything. Its header probes read the build machine's /usr/include,
# which agrees with QNX here (termios, select, sockets, setpgid). The termcap
# interface uses ncurses' termcap API (tgetent and friends in libncursesw,
# <termcap.h> in include/ncursesw), not the -ltermcap of QNX 6.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && sh ./configure --with-x=no --without-gcc --prefix="$PREFIX" qnx6 && \
  sed -i "s!^CC=.*!CC=qcc -V$QCC_TARGET -D_QNX_SOURCE -I$STAGE$PREFIX/include/ncursesw -I$STAGE$PREFIX/include!; \
          s!^LIBS=.*!LIBS=-L$STAGE$PREFIX/lib -lncursesw -lsocket!" Makefile; }
do_build() { make -C "$B" -j6; }
# The install also writes $DESTDIR/etc/elvis/README, a note for an
# administrator's /etc/elvis overrides; a package must not own files in the
# target's /etc, so it goes (elvis still reads /etc/elvis when it exists).
# It runs the just-built elvtags to index data/stubs: on the build machine
# that falls back to the host's ctags, or leaves no tags file, as upstream
# allows ("-" in the Makefile).
do_install() { make -C "$B" DESTDIR="$STAGE" install && rm -rf "$STAGE/etc/elvis" && rmdir "$STAGE/etc" 2>/dev/null; :; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
