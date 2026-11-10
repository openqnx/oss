# unifdef 2.12

> Removes preprocessor conditionals from C/C++ source for given symbols

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://dotat.at/prog/unifdef/unifdef-2.12.tar.xz |
| SHA-256 | `43ce0f02ecdcdc723b2475575563ddb192e988c886d368260bc0a63aee3ac400` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_unifdef`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `unifdef` (programs, shared libraries, run-time data) and `unifdef-dev` (headers, pkg-config/CMake files, static libraries; needs `unifdef`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`unifdef`](https://openqnx.github.io/oss/package/x86_64/unifdef.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/unifdef.html)) · [`unifdef-dev`](https://openqnx.github.io/oss/package/x86_64/unifdef-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/unifdef-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add unifdef` (dependencies come along); one version: `apk add unifdef=<version>` | `apk del unifdef` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install unifdef` (dependencies first) | `oss-get remove unifdef` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`unifdef-dev` the same way, by its own name (`apk add unifdef-dev`, `oss-get install unifdef-dev`, by hand with its own file and list `unifdef-dev.list`); remove it before `unifdef`, which it needs.

```sh
# on the QNX target, as root
apk add unifdef unifdef-dev
apk add qnx-test-unifdef && /usr/local/bin/qnx-port-tests/test_unifdef    # its on-target test, from tests-8.0-<arch>-m-z
apk del unifdef unifdef-dev qnx-test-unifdef
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/unifdef.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/unifdef.list
rm var/lib/manual/unifdef.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild unifdef` performs exactly the same steps (2–5) in one command.

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
URL=https://dotat.at/prog/unifdef/unifdef-2.12.tar.xz
mkdir -p dist src/unifdef
curl -fL -o dist/$(basename $URL) $URL
echo "43ce0f02ecdcdc723b2475575563ddb192e988c886d368260bc0a63aee3ac400  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/unifdef --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/unifdef; B=build/$QNX_ARCH/unifdef; mkdir -p $B
```

```bash
do_build() {
  rm -rf "$B" && mkdir -p "$B" && cp -a "$S/." "$B/" || return 1
  make -C "$B" CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CFLAGS="-O2" unifdef
}
```

```bash
do_install() {
  make -C "$B" prefix="$PREFIX" DESTDIR="$STAGE" install || return 1
  # unifdefall (a #!/bin/sh script) runs "cc -E -dM": it needs a C compiler,
  # which a QNX target does not have; unifdef itself needs nothing.
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- unifdef: removes #ifdef'ed lines from C/C++ source (preprocessor conditionals resolved for given -D/-U symbols, the rest left alone), and unifdefall. Version as in QNX's own package list.
- The release tarball: it carries version.h (the git tree generates it with git describe).
- A plain Makefile that builds in its source tree: build in a copy (src/ stays clean for scripts/mkpatch). Its prefix variable is "prefix" and defaults to $HOME.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH unifdef`.

### 5. What it installs

4 files under `/usr/local`, 2 programs.

```
/usr/local/bin/unifdef
/usr/local/bin/unifdefall
```

The complete list is `stage/<arch>/.manifest/unifdef` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa unifdef
ls dists/8.0/*/ | grep '^unifdef-'
```

This writes `unifdef-<ver>-r0.tar.gz` (run time) and `unifdef-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add unifdef
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/unifdef-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/unifdef-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_unifdef`](../../tests/src/) links unifdef the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-unifdef`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-unifdef
/usr/local/bin/qnx-port-tests/test_unifdef
```

```
== unifdef ==
  ok   ...
PASS unifdef
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_unifdef` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_unifdef`](../../tests/src/) -- a program linked against unifdef the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | unifdef 2.12-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/unifdef-2.12-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | unifdef 2.12-r0 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/unifdef-2.12-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.12**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add unifdef=<version>`), sources in `ported/unifdef/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates unifdef`; port one with `scripts/port-update unifdef`.

<details><summary>recipe.sh</summary>

```bash
# unifdef: removes #ifdef'ed lines from C/C++ source (preprocessor
# conditionals resolved for given -D/-U symbols, the rest left alone), and
# unifdefall. Version as in QNX's own package list.
VERSION=2.12
# The release tarball: it carries version.h (the git tree generates it with
# git describe).
TARBALL=https://dotat.at/prog/unifdef/unifdef-2.12.tar.xz
SHA256=43ce0f02ecdcdc723b2475575563ddb192e988c886d368260bc0a63aee3ac400
DEPS=""
# A plain Makefile that builds in its source tree: build in a copy (src/
# stays clean for scripts/mkpatch). Its prefix variable is "prefix" and
# defaults to $HOME.
BUILDSYS=custom
do_build() {
  rm -rf "$B" && mkdir -p "$B" && cp -a "$S/." "$B/" || return 1
  make -C "$B" CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CFLAGS="-O2" unifdef
}
do_install() {
  make -C "$B" prefix="$PREFIX" DESTDIR="$STAGE" install || return 1
  # unifdefall (a #!/bin/sh script) runs "cc -E -dM": it needs a C compiler,
  # which a QNX target does not have; unifdef itself needs nothing.
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
