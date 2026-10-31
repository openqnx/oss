# fakeroot 1.37.1.1

> Runs a command in an environment faking root privileges for packaging

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://deb.debian.org/debian/pool/main/f/fakeroot/fakeroot_1.37.1.1.orig.tar.gz |
| SHA-256 | `86b0b75bf319ca42e525c098675b6ed10a06b76e69ec9ccf20ef5e03883b3a14` |
| Build system | GNU Autotools |
| QNX patches | 3 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_fakeroot`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `fakeroot` (programs, shared libraries, run-time data) and `fakeroot-dev` (headers, pkg-config/CMake files, static libraries; needs `fakeroot`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`fakeroot`](https://openqnx.github.io/oss/package/x86_64/fakeroot.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/fakeroot.html)) · [`fakeroot-dev`](https://openqnx.github.io/oss/package/x86_64/fakeroot-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/fakeroot-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add fakeroot` (dependencies come along); one version: `apk add fakeroot=<version>` | `apk del fakeroot` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install fakeroot` (dependencies first) | `oss-get remove fakeroot` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`fakeroot-dev` the same way, by its own name (`apk add fakeroot-dev`, `oss-get install fakeroot-dev`, by hand with its own file and list `fakeroot-dev.list`); remove it before `fakeroot`, which it needs.

```sh
# on the QNX target, as root
apk add fakeroot fakeroot-dev
apk add qnx-test-fakeroot && /usr/local/bin/qnx-port-tests/test_fakeroot    # its on-target test
apk del fakeroot fakeroot-dev qnx-test-fakeroot
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/fakeroot.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/fakeroot.list
rm var/lib/manual/fakeroot.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild fakeroot` performs exactly the same steps (2–5) in one command.

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
URL=https://deb.debian.org/debian/pool/main/f/fakeroot/fakeroot_1.37.1.1.orig.tar.gz
mkdir -p dist src/fakeroot
curl -fL -o dist/$(basename $URL) $URL
echo "86b0b75bf319ca42e525c098675b6ed10a06b76e69ec9ccf20ef5e03883b3a14  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/fakeroot --strip-components=1
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/fakeroot < ports/fakeroot/patches/0001-aports-fix-chown.patch
patch -p1 -d src/fakeroot < ports/fakeroot/patches/0002-aports-fix-qnx.patch
patch -p1 -d src/fakeroot < ports/fakeroot/patches/0003-qnx-byte-order-macros.patch
```

**[`0001-aports-fix-chown.patch`](patches/0001-aports-fix-chown.patch)** — QNX change from QNX's aports (core/fakeroot/fix-chown.patch).

> Taken from qnx-ports/aports core/fakeroot -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `fakeroot-1.38.1/libfakeroot.c` · Upstream status: QNX-specific

**[`0002-aports-fix-qnx.patch`](patches/0002-aports-fix-qnx.patch)** — QNX change from QNX's aports (core/fakeroot/fix-qnx.patch).

> Taken from qnx-ports/aports core/fakeroot -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
> Changed here: its message.h hunk (an endian-header case that only exists in
> 1.38) is replaced by 0003 for this release.

Touches: `fakeroot-1.38.1/communicate.c`, `fakeroot-1.38.1/faked.c`, `fakeroot-1.38.1/libfakeroot.c` · Upstream status: QNX-specific

**[`0003-qnx-byte-order-macros.patch`](patches/0003-qnx-byte-order-macros.patch)** — Give message.h the glibc byte-order macro names on QNX.

> message.h picks its 64-bit network byte-order helpers by testing
> __BYTE_ORDER against __BIG_ENDIAN/__LITTLE_ENDIAN, glibc's names. QNX only
> has the BSD spellings in <sys/param.h>; map the glibc names onto the GCC
> predefined macros, which every QNX compiler has.

Touches: `message.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/fakeroot; B=build/$QNX_ARCH/fakeroot; mkdir -p $B
```

```bash
do_install() {
  make -C "$B" DESTDIR="$STAGE" install
  sed -i -e 's|^FAKEROOT_LIB=libfakeroot\.so$|FAKEROOT_LIB=libfakeroot-0.so|' \
         -e '1s|^#!.*|#!/bin/sh|' "$STAGE$PREFIX/bin/fakeroot"
  grep -q '^FAKEROOT_LIB=libfakeroot-0.so$' "$STAGE$PREFIX/bin/fakeroot"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- fakeroot (run as fake root for packaging). Version as in the QNX Everywhere 8.0.4r1 image.
- configure cannot test which IPC works when cross compiling; fakeroot uses its TCP transport on QNX.
- The wrapper preloads FAKEROOT_LIB=libfakeroot.so, the unversioned name that qnxpkg puts in fakeroot-dev (it is the link-time symlink); the runtime package carries only libfakeroot-0.so (libtool -release 0), so `fakeroot` failed on a target with "preload library not found". Point it at the real file. Its shebang is configure's $SHELL -- the build host's /bin/bash, which a QNX target does not have; the script is POSIX sh (Debian runs it so).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH fakeroot`.

### 5. What it installs

20 files under `/usr/local`: 2 shared-library entries, 2 programs.

```
/usr/local/lib/libfakeroot-0.so
/usr/local/lib/libfakeroot.so
/usr/local/bin/faked
/usr/local/bin/fakeroot
```

The complete list is `stage/<arch>/.manifest/fakeroot` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa fakeroot
ls dists/8.0/*/ | grep '^fakeroot-'
```

This writes `fakeroot-<ver>-r0.tar.gz` (run time) and `fakeroot-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add fakeroot
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/fakeroot-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/fakeroot-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_fakeroot`](../../tests/src/) links fakeroot the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-fakeroot`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-fakeroot
/usr/local/bin/qnx-port-tests/test_fakeroot
```

```
== fakeroot ==
  ok   ...
PASS fakeroot
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_fakeroot` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_fakeroot`](../../tests/src/) -- a program linked against fakeroot the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - mkdtemp
   - faked started and printed key:pid
   - getuid/geteuid/getgid/getegid are 0 under fakeroot
   - chown(1234:5678) succeeds and stat reports it
   - mknod(S_IFCHR) succeeds and stat reports a char device
   - second process sees the faked owner (daemon state)
   - second process sees the faked char device
   - real file keeps its real owner
   - real \"device\" is a plain file
   - faked stopped
   - fakeroot wrapper: #!/bin/sh, preloads libfakeroot-0.so
   - fakeroot -- <cmd>: ids are 0

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | fakeroot 1.37.1.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/fakeroot-1.37.1.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | fakeroot 1.37.1.1-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/fakeroot-1.37.1.1-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | fakeroot 1.37.1.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/fakeroot-1.37.1.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | fakeroot 1.37.1.1-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/fakeroot-1.37.1.1-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.37.1.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add fakeroot=<version>`), sources in `ported/fakeroot/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates fakeroot`; port one with `scripts/port-update fakeroot`.

<details><summary>recipe.sh</summary>

```bash
# fakeroot (run as fake root for packaging). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.37.1.1
TARBALL=https://deb.debian.org/debian/pool/main/f/fakeroot/fakeroot_1.37.1.1.orig.tar.gz
SHA256=86b0b75bf319ca42e525c098675b6ed10a06b76e69ec9ccf20ef5e03883b3a14
BUILDSYS=autotools
# configure cannot test which IPC works when cross compiling; fakeroot uses
# its TCP transport on QNX.
CONF_ARGS_EXTRA=( --disable-static --with-ipc=tcp )
# The wrapper preloads FAKEROOT_LIB=libfakeroot.so, the unversioned name that
# qnxpkg puts in fakeroot-dev (it is the link-time symlink); the runtime
# package carries only libfakeroot-0.so (libtool -release 0), so `fakeroot`
# failed on a target with "preload library not found". Point it at the real
# file. Its shebang is configure's $SHELL -- the build host's /bin/bash, which
# a QNX target does not have; the script is POSIX sh (Debian runs it so).
do_install() {
  make -C "$B" DESTDIR="$STAGE" install
  sed -i -e 's|^FAKEROOT_LIB=libfakeroot\.so$|FAKEROOT_LIB=libfakeroot-0.so|' \
         -e '1s|^#!.*|#!/bin/sh|' "$STAGE$PREFIX/bin/fakeroot"
  grep -q '^FAKEROOT_LIB=libfakeroot-0.so$' "$STAGE$PREFIX/bin/fakeroot"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
