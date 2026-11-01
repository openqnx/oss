# man-db 2.13.1

> Manual page reader suite (man, apropos, whatis)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.savannah.gnu.org/releases/man-db/man-db-2.13.1.tar.xz |
| SHA-256 | `8afebb6f7eb6bb8542929458841f5c7e6f240e30c86358c1fbcefbea076c87d9` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libpipeline`](../libpipeline/) [`gdbm`](../gdbm/) [`groff`](../groff/) [`libiconv`](../libiconv/) [`zlib`](../zlib/) |
| Run-time dependencies | [`gdbm`](../gdbm/) [`libiconv`](../libiconv/) [`libpipeline`](../libpipeline/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_man_db`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `man-db` (programs, shared libraries, run-time data) and `man-db-dev` (headers, pkg-config/CMake files, static libraries; needs `man-db`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`man-db`](https://openqnx.github.io/oss/package/x86_64/man-db.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/man-db.html)) · [`man-db-dev`](https://openqnx.github.io/oss/package/x86_64/man-db-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/man-db-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add man-db` (dependencies come along); one version: `apk add man-db=<version>` | `apk del man-db` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install man-db` (dependencies first) | `oss-get remove man-db` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`man-db-dev` the same way, by its own name (`apk add man-db-dev`, `oss-get install man-db-dev`, by hand with its own file and list `man-db-dev.list`); remove it before `man-db`, which it needs.

```sh
# on the QNX target, as root
apk add man-db man-db-dev
apk add qnx-test-man-db && /usr/local/bin/qnx-port-tests/test_man_db    # its on-target test, from tests-8.0-<arch>-m-z
apk del man-db man-db-dev qnx-test-man-db
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/man-db.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/man-db.list
rm var/lib/manual/man-db.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild man-db` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (5 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libpipeline gdbm groff libiconv zlib
```

### 2. Get the upstream source

```bash
URL=https://download.savannah.gnu.org/releases/man-db/man-db-2.13.1.tar.xz
mkdir -p dist src/man-db
curl -fL -o dist/$(basename $URL) $URL
echo "8afebb6f7eb6bb8542929458841f5c7e6f240e30c86358c1fbcefbea076c87d9  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/man-db --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/man-db; B=build/$QNX_ARCH/man-db; mkdir -p $B
```

```bash
do_build()   { _no_manual && make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { _no_manual && make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- man-db, the man page reader. Version as in the QNX Everywhere 8.0.4r1 image.
- Also linked at run time (found by scripts/gen-dependency-index): libiconv zlib
- The "manual" directory typesets man-db's user guide with groff's -me macros on the build host, where they are often not installed (Debian's groff-base lacks them). The guide is not needed to run man; leave that directory out of the generated top-level Makefile. (Not with SUBDIRS=... on the make command line: that reaches every sub-make too, and gl/lib then looks for "docs".)

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH man-db`.

### 5. What it installs

29 files under `/usr/local`, 9 programs.

```
/usr/local/bin/apropos
/usr/local/bin/catman
/usr/local/bin/lexgrog
/usr/local/bin/man
/usr/local/bin/man-recode
/usr/local/bin/mandb
/usr/local/bin/manpath
/usr/local/bin/whatis
/usr/local/sbin/accessdb
```

The complete list is `stage/<arch>/.manifest/man-db` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa man-db
ls dists/8.0/*/ | grep '^man-db-'
```

This writes `man-db-<ver>-r0.tar.gz` (run time) and `man-db-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add man-db
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/man-db-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/man-db-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_man_db`](../../tests/src/) links man-db the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-man-db`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-man-db
/usr/local/bin/qnx-port-tests/test_man_db
```

```
== man-db ==
  ok   ...
PASS man-db
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_man_db` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_man_db`](../../tests/src/) -- a program linked against man-db the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | man-db 2.13.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/man-db-2.13.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | man-db 2.13.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/man-db-2.13.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | man-db 2.13.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/man-db-2.13.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | man-db 2.13.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/man-db-2.13.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.13.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add man-db=<version>`), sources in `ported/man-db/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates man-db`; port one with `scripts/port-update man-db`.

<details><summary>recipe.sh</summary>

```bash
# man-db, the man page reader. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.13.1
TARBALL=https://download.savannah.gnu.org/releases/man-db/man-db-2.13.1.tar.xz
SHA256=8afebb6f7eb6bb8542929458841f5c7e6f240e30c86358c1fbcefbea076c87d9
# Also linked at run time (found by scripts/gen-dependency-index): libiconv zlib
DEPS="libpipeline gdbm groff libiconv zlib"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --disable-setuid --without-libseccomp --disable-cache-owner --with-systemdtmpfilesdir=no --with-systemdsystemunitdir=no )
# The "manual" directory typesets man-db's user guide with groff's -me macros
# on the build host, where they are often not installed (Debian's groff-base
# lacks them). The guide is not needed to run man; leave that directory out of
# the generated top-level Makefile. (Not with SUBDIRS=... on the make command
# line: that reaches every sub-make too, and gl/lib then looks for "docs".)
_no_manual() { sed -i '/^SUBDIRS = /s/ manual / /' "$B/Makefile"; }
do_build()   { _no_manual && make -C "$B" -j"$(nproc)"; }
do_install() { _no_manual && make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
