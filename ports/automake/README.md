# automake 1.18

> GNU tool for generating portable Makefile.in files

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/automake/automake-1.18.tar.xz |
| SHA-256 | `5bdccca96b007a7e344c24204b9b9ac12ecd17f5971931a9063bdee4887f4aaf` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_automake`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add automake
apk add automake-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-automake && /usr/local/bin/qnx-port-tests/test_automake    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install automake`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild automake` performs exactly the same steps (2–5) in one command.

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
URL=https://ftp.gnu.org/gnu/automake/automake-1.18.tar.xz
mkdir -p dist src/automake
curl -fL -o dist/$(basename $URL) $URL
echo "5bdccca96b007a7e344c24204b9b9ac12ecd17f5971931a9063bdee4887f4aaf  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/automake --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/automake; B=build/$QNX_ARCH/automake; mkdir -p $B
```

```bash
do_install() {
  make -C "$B" DESTDIR="$STAGE" install
  local f
  for f in "$STAGE$PREFIX"/bin/*; do sed -i "1s|^#! */usr/bin/perl|#!$PREFIX/bin/perl|" "$f"; done
  # aclocal stops if the system macro directory share/aclocal does not exist.
  # On a target it often does not: it holds only other packages' *.m4 files,
  # which ship in their -dev packages. A missing one is not an error.
  sed -i 's/scan_m4_dirs (FT_SYSTEM, SCAN_M4_DIRS_ERROR, @system_includes);/scan_m4_dirs (FT_SYSTEM, SCAN_M4_DIRS_SILENT, @system_includes);/' \
    "$STAGE$PREFIX/bin/aclocal" "$STAGE$PREFIX/bin/aclocal-1.18"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GNU Automake. Version as in the QNX Everywhere 8.0.4r1 image.
- aclocal and automake are generated with the build machine's perl path (#!/usr/bin/perl), which a QNX target does not have (hazard 105).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH automake`.

### 5. What it installs

125 files under `/usr/local`, 4 programs.

```
/usr/local/bin/aclocal
/usr/local/bin/aclocal-1.18
/usr/local/bin/automake
/usr/local/bin/automake-1.18
```

The complete list is `stage/<arch>/.manifest/automake` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa automake
ls dists/8.0/*/ | grep '^automake-'
```

This writes `automake-<ver>-r0.tar.gz` (run time) and `automake-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add automake
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/automake-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/automake-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_automake`](../../tests/src/) links automake the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-automake`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-automake
/usr/local/bin/qnx-port-tests/test_automake
```

```
== automake ==
  ok   ...
PASS automake
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_automake` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_automake`](../../tests/src/) -- a program linked against automake the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | automake 1.18-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/automake-1.18-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | automake 1.18-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/automake-1.18-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | automake 1.18-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/automake-1.18-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | automake 1.18-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/automake-1.18-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.18**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add automake=<version>`), sources in `ported/automake/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates automake`; port one with `scripts/port-update automake`.

<details><summary>recipe.sh</summary>

```bash
# GNU Automake. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.18
TARBALL=https://ftp.gnu.org/gnu/automake/automake-1.18.tar.xz
SHA256=5bdccca96b007a7e344c24204b9b9ac12ecd17f5971931a9063bdee4887f4aaf
BUILDSYS=autotools
CONF_ARGS_EXTRA=(  )

# aclocal and automake are generated with the build machine's perl path
# (#!/usr/bin/perl), which a QNX target does not have (hazard 105).
RUNTIME_DEPS="perl autoconf"
do_install() {
  make -C "$B" DESTDIR="$STAGE" install
  local f
  for f in "$STAGE$PREFIX"/bin/*; do sed -i "1s|^#! */usr/bin/perl|#!$PREFIX/bin/perl|" "$f"; done
  # aclocal stops if the system macro directory share/aclocal does not exist.
  # On a target it often does not: it holds only other packages' *.m4 files,
  # which ship in their -dev packages. A missing one is not an error.
  sed -i 's/scan_m4_dirs (FT_SYSTEM, SCAN_M4_DIRS_ERROR, @system_includes);/scan_m4_dirs (FT_SYSTEM, SCAN_M4_DIRS_SILENT, @system_includes);/' \
    "$STAGE$PREFIX/bin/aclocal" "$STAGE$PREFIX/bin/aclocal-1.18"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
