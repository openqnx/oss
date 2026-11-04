# help2man 1.49.3

> Generates man pages from a program's --help and --version output

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/help2man/help2man-1.49.3.tar.xz |
| SHA-256 | `4d7e4fdef2eca6afe07a2682151cea78781e0a4e8f9622142d9f70c083a2fd4f` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_help2man`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `help2man` (programs, shared libraries, run-time data) and `help2man-dev` (headers, pkg-config/CMake files, static libraries; needs `help2man`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`help2man`](https://openqnx.github.io/oss/package/x86_64/help2man.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/help2man.html)) · [`help2man-dev`](https://openqnx.github.io/oss/package/x86_64/help2man-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/help2man-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add help2man` (dependencies come along); one version: `apk add help2man=<version>` | `apk del help2man` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install help2man` (dependencies first) | `oss-get remove help2man` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`help2man-dev` the same way, by its own name (`apk add help2man-dev`, `oss-get install help2man-dev`, by hand with its own file and list `help2man-dev.list`); remove it before `help2man`, which it needs.

```sh
# on the QNX target, as root
apk add help2man help2man-dev
apk add qnx-test-help2man && /usr/local/bin/qnx-port-tests/test_help2man    # its on-target test, from tests-8.0-<arch>
apk del help2man help2man-dev qnx-test-help2man
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/help2man.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/help2man.list
rm var/lib/manual/help2man.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild help2man` performs exactly the same steps (2–5) in one command.

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
URL=https://ftp.gnu.org/gnu/help2man/help2man-1.49.3.tar.xz
mkdir -p dist src/help2man
curl -fL -o dist/$(basename $URL) $URL
echo "4d7e4fdef2eca6afe07a2682151cea78781e0a4e8f9622142d9f70c083a2fd4f  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/help2man --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/help2man; B=build/$QNX_ARCH/help2man; mkdir -p $B
```

```bash
do_install() {
  make -C "$B" DESTDIR="$STAGE" install || return 1
  # #!/usr/bin/perl is the build machine's (hazard 105).
  sed -i "1s|^#! */usr/bin/perl|#!$PREFIX/bin/perl|" "$STAGE$PREFIX/bin/help2man"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- help2man: generates a man page from a program's --help and --version output. A Perl script. Version as in QNX's own package list.
- --disable-nls: otherwise configure finds the build machine perl's Locale::gettext and generates help2man "--with-gettext": the script then requires Locale::gettext, which the perl port does not have, and installs bindtextdomain.so, an LD_PRELOAD helper for --locale. Without NLS it is a plain script that needs only perl's core modules.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH help2man`.

### 5. What it installs

3 files under `/usr/local`, 1 programs.

```
/usr/local/bin/help2man
```

The complete list is `stage/<arch>/.manifest/help2man` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa help2man
ls dists/8.0/*/ | grep '^help2man-'
```

This writes `help2man-<ver>-r0.tar.gz` (run time) and `help2man-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add help2man
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/help2man-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/help2man-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_help2man`](../../tests/src/) links help2man the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-help2man`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-help2man
/usr/local/bin/qnx-port-tests/test_help2man
```

```
== help2man ==
  ok   ...
PASS help2man
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_help2man` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_help2man`](../../tests/src/) -- a program linked against help2man the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> help2man`.

## Versions

This recipe builds **1.49.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add help2man=<version>`), sources in `ported/help2man/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates help2man`; port one with `scripts/port-update help2man`.

<details><summary>recipe.sh</summary>

```bash
# help2man: generates a man page from a program's --help and --version
# output. A Perl script. Version as in QNX's own package list.
VERSION=1.49.3
TARBALL=https://ftp.gnu.org/gnu/help2man/help2man-1.49.3.tar.xz
SHA256=4d7e4fdef2eca6afe07a2682151cea78781e0a4e8f9622142d9f70c083a2fd4f
DEPS=""
RUNTIME_DEPS="perl"
BUILDSYS=autotools
# --disable-nls: otherwise configure finds the build machine perl's
# Locale::gettext and generates help2man "--with-gettext": the script then
# requires Locale::gettext, which the perl port does not have, and installs
# bindtextdomain.so, an LD_PRELOAD helper for --locale. Without NLS it is a
# plain script that needs only perl's core modules.
CONF_ARGS_EXTRA=( --disable-nls )
do_install() {
  make -C "$B" DESTDIR="$STAGE" install || return 1
  # #!/usr/bin/perl is the build machine's (hazard 105).
  sed -i "1s|^#! */usr/bin/perl|#!$PREFIX/bin/perl|" "$STAGE$PREFIX/bin/help2man"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
