# fio 3.41

> Flexible I/O tester: storage benchmark and data verification with job files

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/axboe/fio.git (tag `fio-3.41`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`zlib`](../zlib/) |
| Run-time dependencies | [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_fio`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `fio` (programs, shared libraries, run-time data) and `fio-dev` (headers, pkg-config/CMake files, static libraries; needs `fio`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`fio`](https://openqnx.github.io/oss/package/x86_64/fio.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/fio.html)) · [`fio-dev`](https://openqnx.github.io/oss/package/x86_64/fio-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/fio-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add fio` (dependencies come along); one version: `apk add fio=<version>` | `apk del fio` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install fio` (dependencies first) | `oss-get remove fio` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`fio-dev` the same way, by its own name (`apk add fio-dev`, `oss-get install fio-dev`, by hand with its own file and list `fio-dev.list`); remove it before `fio`, which it needs.

```sh
# on the QNX target, as root
apk add fio fio-dev
apk add qnx-test-fio && /usr/local/bin/qnx-port-tests/test_fio    # its on-target test, from tests-8.0-<arch>
apk del fio fio-dev qnx-test-fio
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/fio.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/fio.list
rm var/lib/manual/fio.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild fio` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild zlib
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b fio-3.41 --recurse-submodules --shallow-submodules \
    https://github.com/axboe/fio.git src/fio
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/fio < ports/fio/patches/0001-qcc-dependency-files.patch
```

**[`0001-qcc-dependency-files.patch`](patches/0001-qcc-dependency-files.patch)** — Generate dependency files with -E -Wp,-MM: qcc links on a bare -MM.

> fio's Makefile writes each object's dependency file with "$(CC) -MM".
> QNX's compiler driver qcc does not treat -MM as "preprocess only" the way
> gcc does: it compiles and then links the file, so every object fails with
> "undefined reference to main" from crt1.o. "-E -Wp,-MM" asks for the
> preprocessor explicitly and hands -MM to it; gcc and clang produce the same
> dependency output with it, so the change is portable.

Touches: `Makefile` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/fio; B=build/$QNX_ARCH/fio; mkdir -p $B
```

```bash
do_configure() {
  # fio's configure and Makefile work in the source tree only: build in a
  # copy of it, so src/fio stays clean (scripts/mkpatch diffs it). The
  # staging include directory comes first: zlib.h must be the zlib port's,
  # the library fio links.
  rm -rf "$B" && mkdir -p "$B" && cp -a "$S/." "$B/" && rm -rf "$B/.git" || return 1
  cd "$B" && ./configure --prefix="$PREFIX" \
    --cc="qcc -V$QCC_TARGET" --extra-cflags="-D_QNX_SOURCE -I$STAGE_SYSROOT$PREFIX/include $CFLAGS" \
    --disable-native --disable-shm --disable-http 2>&1 | tail -n 80
}
```

```bash
do_build() { make -C "$B" -j"$(nproc)" V=1; }
```

```bash
do_install() {
  make -C "$B" DESTDIR="$STAGE" prefix="$PREFIX" install || return 1
  # genfio starts with #!/bin/bash; a QNX target's bash is the bash port's.
  sed -i "1s|^#!/bin/bash|#!$PREFIX/bin/bash|" "$STAGE$PREFIX/bin/genfio"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- fio: the flexible I/O tester -- benchmarks and verifies storage with job files (sequential/random reads and writes, many I/O engines, data verification). Version as in QNX's own package list.
- zlib: compressed I/O logs (log_compression) and the zlib buffer compression test. Nothing else from staging: --disable-http leaves out the http (WebDAV/S3/Swift) engine, which would link curl and OpenSSL.
- The helper scripts (fio2gnuplot, fiologparser*.py, fio_jsonplus_clat2csv, ...) are Python and genfio is bash: they need python3 / bash on the target, which fio itself does not; they are not declared as dependencies. fio has its own configure (not Autoconf) and already knows QNX (os/os-qnx.h): it detects the target from the compiler (__QNX__), so it is given qcc and the target's flags; --disable-native keeps -march=native (the build machine's CPU) out of a cross build. --disable-shm: QNX has no System V shared memory (no <sys/shm.h>, no shmget), which fio uses to share job state between forked job processes. Without it fio runs every job as a thread (thread=1 is the default then) and has no client/server mode (--server, --client).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH fio`.

### 5. What it installs

18 files under `/usr/local`, 11 programs.

```
/usr/local/bin/fio
/usr/local/bin/fio-dedupe
/usr/local/bin/fio-genzipf
/usr/local/bin/fio-histo-log-pctiles.py
/usr/local/bin/fio-verify-state
/usr/local/bin/fio2gnuplot
/usr/local/bin/fio_generate_plots
/usr/local/bin/fio_jsonplus_clat2csv
/usr/local/bin/fiologparser.py
/usr/local/bin/fiologparser_hist.py
/usr/local/bin/genfio
```

The complete list is `stage/<arch>/.manifest/fio` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa fio
ls dists/8.0/*/ | grep '^fio-'
```

This writes `fio-<ver>-r0.tar.gz` (run time) and `fio-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add fio
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/fio-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/fio-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_fio`](../../tests/src/) links fio the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-fio`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-fio
/usr/local/bin/qnx-port-tests/test_fio
```

```
== fio ==
  ok   ...
PASS fio
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_fio` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_fio`](../../tests/src/) -- a program linked against fio the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> fio`.

## Versions

This recipe builds **3.41**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add fio=<version>`), sources in `ported/fio/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates fio`; port one with `scripts/port-update fio`.

<details><summary>recipe.sh</summary>

```bash
# fio: the flexible I/O tester -- benchmarks and verifies storage with job
# files (sequential/random reads and writes, many I/O engines, data
# verification). Version as in QNX's own package list.
VERSION=3.41; GIT=https://github.com/axboe/fio.git; REF=fio-3.41
# zlib: compressed I/O logs (log_compression) and the zlib buffer
# compression test. Nothing else from staging: --disable-http leaves out the
# http (WebDAV/S3/Swift) engine, which would link curl and OpenSSL.
DEPS="zlib"
# The helper scripts (fio2gnuplot, fiologparser*.py, fio_jsonplus_clat2csv,
# ...) are Python and genfio is bash: they need python3 / bash on the target,
# which fio itself does not; they are not declared as dependencies.
# fio has its own configure (not Autoconf) and already knows QNX
# (os/os-qnx.h): it detects the target from the compiler (__QNX__), so it is
# given qcc and the target's flags; --disable-native keeps -march=native (the
# build machine's CPU) out of a cross build.
# --disable-shm: QNX has no System V shared memory (no <sys/shm.h>, no
# shmget), which fio uses to share job state between forked job processes.
# Without it fio runs every job as a thread (thread=1 is the default then)
# and has no client/server mode (--server, --client).
BUILDSYS=custom
do_configure() {
  # fio's configure and Makefile work in the source tree only: build in a
  # copy of it, so src/fio stays clean (scripts/mkpatch diffs it). The
  # staging include directory comes first: zlib.h must be the zlib port's,
  # the library fio links.
  rm -rf "$B" && mkdir -p "$B" && cp -a "$S/." "$B/" && rm -rf "$B/.git" || return 1
  cd "$B" && ./configure --prefix="$PREFIX" \
    --cc="qcc -V$QCC_TARGET" --extra-cflags="-D_QNX_SOURCE -I$STAGE_SYSROOT$PREFIX/include $CFLAGS" \
    --disable-native --disable-shm --disable-http 2>&1 | tail -n 80
}
do_build() { make -C "$B" -j"$(nproc)" V=1; }
do_install() {
  make -C "$B" DESTDIR="$STAGE" prefix="$PREFIX" install || return 1
  # genfio starts with #!/bin/bash; a QNX target's bash is the bash port's.
  sed -i "1s|^#!/bin/bash|#!$PREFIX/bin/bash|" "$STAGE$PREFIX/bin/genfio"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
