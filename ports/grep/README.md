# grep 3.11

> GNU grep, searches files for lines matching a pattern

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/grep/grep-3.11.tar.xz |
| SHA-256 | `1db2aedde89d0dea42b16d9528f894c8d15dae4e190b59aecc78f5a951276eab` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`pcre2`](../pcre2/) [`libiconv`](../libiconv/) |
| Run-time dependencies | [`libiconv`](../libiconv/) [`pcre2`](../pcre2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_grep`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `grep` (programs, shared libraries, run-time data) and `grep-dev` (headers, pkg-config/CMake files, static libraries; needs `grep`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`grep`](https://openqnx.github.io/oss/package/x86_64/grep.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/grep.html)) · [`grep-dev`](https://openqnx.github.io/oss/package/x86_64/grep-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/grep-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add grep` (dependencies come along); one version: `apk add grep=<version>` | `apk del grep` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install grep` (dependencies first) | `oss-get remove grep` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`grep-dev` the same way, by its own name (`apk add grep-dev`, `oss-get install grep-dev`, by hand with its own file and list `grep-dev.list`); remove it before `grep`, which it needs.

```sh
# on the QNX target, as root
apk add grep grep-dev
apk add qnx-test-grep && /usr/local/bin/qnx-port-tests/test_grep    # its on-target test, from tests-8.0-<arch>
apk del grep grep-dev qnx-test-grep
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/grep.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/grep.list
rm var/lib/manual/grep.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild grep` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild pcre2 libiconv
```

<details><summary>Full build chain (4)</summary>

[`bzip2`](../bzip2/) [`libiconv`](../libiconv/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/grep/grep-3.11.tar.xz
mkdir -p dist src/grep
curl -fL -o dist/$(basename $URL) $URL
echo "1db2aedde89d0dea42b16d9528f894c8d15dae4e190b59aecc78f5a951276eab  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/grep --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/grep; B=build/$QNX_ARCH/grep; mkdir -p $B
```

```bash
do_install() {
  make -C "$B" DESTDIR="$STAGE" install
  sed -i '1s|^#!.*|#!/bin/sh|' "$STAGE$PREFIX/bin/egrep" "$STAGE$PREFIX/bin/fgrep"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GNU grep. Version as in the QNX Everywhere 8.0.4r1 image.
- Also linked at run time (found by scripts/gen-dependency-index): libiconv
- egrep and fgrep are #!/bin/bash wrappers (configure's SHELL); a QNX target has no /bin/bash, and they shadow the base system's egrep/fgrep in PATH. They are POSIX sh scripts: run them with /bin/sh (hazard 105).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH grep`.

### 5. What it installs

5 files under `/usr/local`, 3 programs.

```
/usr/local/bin/egrep
/usr/local/bin/fgrep
/usr/local/bin/grep
```

The complete list is `stage/<arch>/.manifest/grep` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa grep
ls dists/8.0/*/ | grep '^grep-'
```

This writes `grep-<ver>-r0.tar.gz` (run time) and `grep-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add grep
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/grep-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/grep-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_grep`](../../tests/src/) links grep the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-grep`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-grep
/usr/local/bin/qnx-port-tests/test_grep
```

```
== grep ==
  ok   ...
PASS grep
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_grep` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_grep`](../../tests/src/) -- a program linked against grep the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | grep 3.11-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/grep-3.11-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | grep 3.11-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/grep-3.11-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | grep 3.11-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/grep-3.11-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | grep 3.11-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/grep-3.11-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.11**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add grep=<version>`), sources in `ported/grep/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates grep`; port one with `scripts/port-update grep`.

<details><summary>recipe.sh</summary>

```bash
# GNU grep. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=3.11
TARBALL=https://ftp.gnu.org/gnu/grep/grep-3.11.tar.xz
SHA256=1db2aedde89d0dea42b16d9528f894c8d15dae4e190b59aecc78f5a951276eab
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="pcre2 libiconv"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls )

# egrep and fgrep are #!/bin/bash wrappers (configure's SHELL); a QNX target
# has no /bin/bash, and they shadow the base system's egrep/fgrep in PATH.
# They are POSIX sh scripts: run them with /bin/sh (hazard 105).
do_install() {
  make -C "$B" DESTDIR="$STAGE" install
  sed -i '1s|^#!.*|#!/bin/sh|' "$STAGE$PREFIX/bin/egrep" "$STAGE$PREFIX/bin/fgrep"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
