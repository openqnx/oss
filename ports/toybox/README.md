# toybox 0.8.15

> Many common Linux command line utilities in a single BSD-licensed binary

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://landley.net/toybox/downloads/toybox-0.8.15.tar.gz |
| SHA-256 | `44e2bab43dfe2c171743297631ce558c6d9ed38b87a66fd751cef617574b375e` |
| Build system | custom (see recipe) |
| QNX patches | 6 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_toybox`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `toybox`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`toybox`](https://openqnx.github.io/oss/package/x86_64/toybox.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/toybox.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add toybox` (dependencies come along); one version: `apk add toybox=<version>` | `apk del toybox` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install toybox` (dependencies first) | `oss-get remove toybox` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-toybox && /usr/local/bin/qnx-port-tests/test_toybox    # its on-target test, from tests-8.0-<arch>-m-z
apk del toybox qnx-test-toybox
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/toybox.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/toybox.list
rm var/lib/manual/toybox.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild toybox` performs exactly the same steps (2–5) in one command.

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
URL=https://landley.net/toybox/downloads/toybox-0.8.15.tar.gz
mkdir -p dist src/toybox
curl -fL -o dist/$(basename $URL) $URL
echo "44e2bab43dfe2c171743297631ce558c6d9ed38b87a66fd751cef617574b375e  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/toybox --strip-components=1
```

### 3. Apply the QNX patches

6 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/toybox < ports/toybox/patches/0001-qnx-portability.patch
patch -p1 -d src/toybox < ports/toybox/patches/0002-qnx-dirtree-no-d-type.patch
patch -p1 -d src/toybox < ports/toybox/patches/0003-qnx-no-uselocale.patch
patch -p1 -d src/toybox < ports/toybox/patches/0004-qnx-stat-no-f-type.patch
patch -p1 -d src/toybox < ports/toybox/patches/0005-qnx-o-directory.patch
patch -p1 -d src/toybox < ports/toybox/patches/0006-qnx-open-directories.patch
```

**[`0001-qnx-portability.patch`](patches/0001-qnx-portability.patch)** — Add QNX to the portability layer.

> toybox's portability layer knows Linux, macOS and the BSDs and stops with
> #error anywhere else. The QNX branch: statvfs() under the statfs names
> (QNX has no statfs; its struct statvfs carries the filesystem type name,
> f_basetype); no mount table interface (xgetmountlist returns nothing: df
> and mount are not built); inotify for tail -f through QNX's emulation
> (libfsnotify); no extended attributes (the xattr_* calls fail with
> ENOTSUP); major()/minor() and QNX's three-argument makedev(); forkpty()
> from <unix.h>; no <sys/syscall.h> or <sys/ttydefaults.h> (and a syscall()
> stub for the one caller, copy_file_range, whose number QNX does not
> have); SA_RESTART (unsupported) as 0; a default _PATH_DEFPATH.

Touches: `lib/portability.c`, `lib/portability.h`, `toys.h` · Upstream status: QNX-specific

**[`0002-qnx-dirtree-no-d-type.patch`](patches/0002-qnx-dirtree-no-d-type.patch)** — dirtree: use lstat information where dirent has no d_type.

> QNX's struct dirent has no d_type. dirtree used it to notice symlinks
> (for -L/-H style following) and, for entries it did not stat, as the file
> type; on QNX both come from fstatat(AT_SYMLINK_NOFOLLOW).

Touches: `lib/dirtree.c` · Upstream status: QNX-specific

**[`0003-qnx-no-uselocale.patch`](patches/0003-qnx-no-uselocale.patch)** — Do not merge a UTF-8 locale on QNX.

> QNX has only the C locale and no newlocale()/uselocale() (no xlocale):
> skip merging in a UTF-8 locale's character types.

Touches: `main.c` · Upstream status: QNX-specific

**[`0004-qnx-stat-no-f-type.patch`](patches/0004-qnx-stat-no-f-type.patch)** — stat -f: no numeric filesystem type on QNX.

> QNX's struct statvfs (toybox's statfs there, see 0001) has the filesystem
> type as a name, f_basetype, which %T prints, but no numeric f_type:
> %t prints 0.

Touches: `toys/other/stat.c` · Upstream status: QNX-specific

**[`0005-qnx-o-directory.patch`](patches/0005-qnx-o-directory.patch)** — Open directories with O_DIRECTORY; no O_PATH/O_NOATIME on QNX.

> On QNX, openat() under a directory descriptor that was opened without
> O_DIRECTORY fails with ENOTDIR (and fdopendir() of it too), so every
> recursive walk (find, ls -R, cp -r, tar, rm -r) stopped one level down:
> "find: .: Not a directory". dirtree opens only directories here, so it
> says so. xabspath() (realpath, readlink -f) likewise opens the directories
> it resolves through with O_DIRECTORY. And toybox's fallback values for
> O_PATH and O_NOATIME (010000000, 01000000) are QNX's O_DIRFD and O_ANON:
> on QNX they are 0.

Touches: `lib/dirtree.c`, `lib/portability.h`, `lib/xwrap.c` · Upstream status: QNX-specific

**[`0006-qnx-open-directories.patch`](patches/0006-qnx-open-directories.patch)** — Reopen directories with O_DIRECTORY on QNX, for every open()/openat().

> Patch 0005 gave dirtree's own directory opens O_DIRECTORY, but toybox opens
> directories it then uses with *at() in many more places: ls (open() and
> openat() of each listed directory: "ls: d/a: Not a directory"), cp -r (the
> new target directory: "cp: e/a: Not a directory"), and others. On QNX,
> open()/openat() are wrapped: a directory opened read-only without
> O_DIRECTORY is reopened with it (same descriptor number, same
> close-on-exec flag), so every caller gets a descriptor *at() works with.

Touches: `lib/portability.c`, `lib/portability.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/toybox; B=build/$QNX_ARCH/toybox; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/" && cd "$B" && \
  KCONFIG_ALLCONFIG="$RECIPE_DIR/toybox/miniconfig" make allnoconfig HOSTCC=gcc; }
```

```bash
do_build() { cd "$B" && CROSS_COMPILE="$QNX_HOST/usr/bin/$QNX_TRIPLE-" CC=gcc HOSTCC=gcc \
  STRIP="$QNX_HOST/usr/bin/nto${QNX_CPU}-strip" CFLAGS="-D_QNX_SOURCE" LDFLAGS="-lregex -lsocket -lfsnotify" \
  CPUS=6 V=1 scripts/make.sh; }
```

```bash
do_install() { install -Dm755 "$B/toybox" "$STAGE$PREFIX/opt/toybox/bin/toybox"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Upstream toybox (landley.net), as one multicall binary with no applet links, in a prefix of its own: /usr/local/opt/toybox/bin/toybox. QNX's base image ships its own toybox, whose names this must not take over; run an applet as "toybox <applet>" (or link it yourself).
- The applets: miniconfig (macOS's set, less the ones that need a mount table, utmpx or Linux /proc); kconfig itself runs on the build machine (HOSTCC). The QNX patches add a QNX branch to the portability layer (statvfs for statfs, no xattr, no Linux syscall numbers, inotify via libfsnotify), and replace dirent d_type and the locale merging.
- The cross-GCC driver (toybox's make.sh composes its own command lines); regex is in libregex, sockets in libsocket, inotify in libfsnotify. No zlib/libcrypto (the config leaves TOYBOX_LIBZ/LIBCRYPTO off: the SDP's copies would be found).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH toybox`.

### 5. What it installs

1 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/toybox` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa toybox
ls dists/8.0/*/ | grep '^toybox-'
```

This writes `toybox-<ver>-r0.tar.gz` (run time) and `toybox-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add toybox
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/toybox-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/toybox-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_toybox`](../../tests/src/) links toybox the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-toybox`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-toybox
/usr/local/bin/qnx-port-tests/test_toybox
```

```
== toybox ==
  ok   ...
PASS toybox
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_toybox` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_toybox`](../../tests/src/) -- a program linked against toybox the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | toybox 0.8.15-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/toybox-0.8.15-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | toybox 0.8.15-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/toybox-0.8.15-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.8.15**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add toybox=<version>`), sources in `ported/toybox/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates toybox`; port one with `scripts/port-update toybox`.

<details><summary>recipe.sh</summary>

```bash
# Upstream toybox (landley.net), as one multicall binary with no applet
# links, in a prefix of its own: /usr/local/opt/toybox/bin/toybox. QNX's
# base image ships its own toybox, whose names this must not take over;
# run an applet as "toybox <applet>" (or link it yourself).
VERSION=0.8.15
TARBALL=https://landley.net/toybox/downloads/toybox-0.8.15.tar.gz
SHA256=44e2bab43dfe2c171743297631ce558c6d9ed38b87a66fd751cef617574b375e
BUILDSYS=custom
# The applets: miniconfig (macOS's set, less the ones that need a
# mount table, utmpx or Linux /proc); kconfig itself runs on the build
# machine (HOSTCC). The QNX patches add a QNX branch to the portability
# layer (statvfs for statfs, no xattr, no Linux syscall numbers, inotify via
# libfsnotify), and replace dirent d_type and the locale merging.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && \
  KCONFIG_ALLCONFIG="$RECIPE_DIR/toybox/miniconfig" make allnoconfig HOSTCC=gcc; }
# The cross-GCC driver (toybox's make.sh composes its own command lines);
# regex is in libregex, sockets in libsocket, inotify in libfsnotify. No zlib/libcrypto (the
# config leaves TOYBOX_LIBZ/LIBCRYPTO off: the SDP's copies would be found).
do_build() { cd "$B" && CROSS_COMPILE="$QNX_HOST/usr/bin/$QNX_TRIPLE-" CC=gcc HOSTCC=gcc \
  STRIP="$QNX_HOST/usr/bin/nto${QNX_CPU}-strip" CFLAGS="-D_QNX_SOURCE" LDFLAGS="-lregex -lsocket -lfsnotify" \
  CPUS=6 V=1 scripts/make.sh; }
do_install() { install -Dm755 "$B/toybox" "$STAGE$PREFIX/opt/toybox/bin/toybox"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
