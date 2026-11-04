# datefudge 1.24

> Runs a program with a faked current time (LD_PRELOAD library and wrapper)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://deb.debian.org/debian/pool/main/d/datefudge/datefudge_1.24.tar.xz |
| SHA-256 | `80efab249232deeb142bf8a0ec66f2cf596c45560e4fdd28f3b5bdd290ac0385` |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_datefudge`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `datefudge` (programs, shared libraries, run-time data) and `datefudge-dev` (headers, pkg-config/CMake files, static libraries; needs `datefudge`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`datefudge`](https://openqnx.github.io/oss/package/x86_64/datefudge.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/datefudge.html)) · [`datefudge-dev`](https://openqnx.github.io/oss/package/x86_64/datefudge-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/datefudge-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add datefudge` (dependencies come along); one version: `apk add datefudge=<version>` | `apk del datefudge` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install datefudge` (dependencies first) | `oss-get remove datefudge` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`datefudge-dev` the same way, by its own name (`apk add datefudge-dev`, `oss-get install datefudge-dev`, by hand with its own file and list `datefudge-dev.list`); remove it before `datefudge`, which it needs.

```sh
# on the QNX target, as root
apk add datefudge datefudge-dev
apk add qnx-test-datefudge && /usr/local/bin/qnx-port-tests/test_datefudge    # its on-target test, from tests-8.0-<arch>
apk del datefudge datefudge-dev qnx-test-datefudge
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/datefudge.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/datefudge.list
rm var/lib/manual/datefudge.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild datefudge` performs exactly the same steps (2–5) in one command.

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
URL=https://deb.debian.org/debian/pool/main/d/datefudge/datefudge_1.24.tar.xz
mkdir -p dist src/datefudge
curl -fL -o dist/$(basename $URL) $URL
echo "80efab249232deeb142bf8a0ec66f2cf596c45560e4fdd28f3b5bdd290ac0385  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/datefudge --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/datefudge < ports/datefudge/patches/0001-qnx-no-features-h.patch
patch -p1 -d src/datefudge < ports/datefudge/patches/0002-qnx-gettimeofday-lookup.patch
```

**[`0001-qnx-no-features-h.patch`](patches/0001-qnx-no-features-h.patch)** — Do not include glibc's <features.h> on QNX.

> <features.h> is glibc's; QNX has none (its feature macros come with every
> standard header). datefudge includes it for nothing it uses.

Touches: `datefudge.c` · Upstream status: QNX-specific

**[`0002-qnx-gettimeofday-lookup.patch`](patches/0002-qnx-gettimeofday-lookup.patch)** — Look up libc's gettimeofday, not glibc's __gettimeofday, on QNX.

> datefudge's gettimeofday() calls __gettimeofday(), which finds the real
> function with dlsym(RTLD_NEXT, "__gettimeofday") -- glibc's internal alias.
> QNX's libc exports only gettimeofday, so the lookup returns NULL and every
> program that calls gettimeofday() under datefudge jumps to address 0. On
> QNX the next definition after the preloaded library is libc's
> gettimeofday itself.

Touches: `datefudge.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/datefudge; B=build/$QNX_ARCH/datefudge; mkdir -p $B
```

```bash
do_build() {
  rm -rf "$B" && mkdir -p "$B" && cp -a "$S/." "$B/" || return 1
  make -C "$B" VERSION=$VERSION prefix="$PREFIX" datefudge datefudge.1 || return 1
  qcc -V$QCC_TARGET -D_QNX_SOURCE -O2 -Wall -Wextra -fpic -shared -o "$B/datefudge.so" "$B/datefudge.c"
}
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/lib/datefudge" "$STAGE$PREFIX/bin" "$STAGE$PREFIX/share/man/man1" || return 1
  install -m 644 "$B/datefudge.so" "$STAGE$PREFIX/lib/datefudge/datefudge.so" &&
  install -m 755 "$B/datefudge" "$STAGE$PREFIX/bin/datefudge" &&
  install -m 644 "$B/datefudge.1" "$STAGE$PREFIX/share/man/man1/datefudge.1"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- datefudge: runs a program with a faked current time -- an LD_PRELOAD library that shifts what time(), gettimeofday() and clock_gettime() (CLOCK_REALTIME) return, and the datefudge wrapper script. Version as in QNX's own package list (Debian's upstream).
- QNX's runtime linker honours LD_PRELOAD, and a preloaded library's definitions interpose on the program's calls as on Linux. Calls libc makes internally (strftime, localtime, ...) are not affected -- the same as with glibc. The wrapper runs "date -d" to parse the date: QNX's /system/bin/date (toybox) accepts it.
- The Makefile builds in its directory: a copy. VERSION= because it would take the version from dpkg-parsechangelog. The wrapper (datefudge.sh with its paths filled in) and the manual come from the Makefile; the library is linked here: the Makefile links it with -ldl, and QNX has dlopen/dlsym in libc and no libdl. It goes to $PREFIX/lib/datefudge/datefudge.so, where the installed wrapper looks for it.
- Installed by hand: "make install" depends on datefudge.o, which this build never makes, so make would compile and link datefudge.so again -- with the build machine's cc (a Linux library in the package).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH datefudge`.

### 5. What it installs

3 files under `/usr/local`, 1 programs.

```
/usr/local/bin/datefudge
```

The complete list is `stage/<arch>/.manifest/datefudge` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa datefudge
ls dists/8.0/*/ | grep '^datefudge-'
```

This writes `datefudge-<ver>-r0.tar.gz` (run time) and `datefudge-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add datefudge
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/datefudge-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/datefudge-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_datefudge`](../../tests/src/) links datefudge the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-datefudge`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-datefudge
/usr/local/bin/qnx-port-tests/test_datefudge
```

```
== datefudge ==
  ok   ...
PASS datefudge
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_datefudge` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_datefudge`](../../tests/src/) -- a program linked against datefudge the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> datefudge`.

## Versions

This recipe builds **1.24**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add datefudge=<version>`), sources in `ported/datefudge/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates datefudge`; port one with `scripts/port-update datefudge`.

<details><summary>recipe.sh</summary>

```bash
# datefudge: runs a program with a faked current time -- an LD_PRELOAD
# library that shifts what time(), gettimeofday() and clock_gettime()
# (CLOCK_REALTIME) return, and the datefudge wrapper script. Version as in
# QNX's own package list (Debian's upstream).
VERSION=1.24
TARBALL=https://deb.debian.org/debian/pool/main/d/datefudge/datefudge_1.24.tar.xz
SHA256=80efab249232deeb142bf8a0ec66f2cf596c45560e4fdd28f3b5bdd290ac0385
DEPS=""
# QNX's runtime linker honours LD_PRELOAD, and a preloaded library's
# definitions interpose on the program's calls as on Linux. Calls libc
# makes internally (strftime, localtime, ...) are not affected -- the same
# as with glibc. The wrapper runs "date -d" to parse the date: QNX's
# /system/bin/date (toybox) accepts it.
BUILDSYS=custom
# The Makefile builds in its directory: a copy. VERSION= because it would
# take the version from dpkg-parsechangelog. The wrapper (datefudge.sh with
# its paths filled in) and the manual come from the Makefile; the library is
# linked here: the Makefile links it with -ldl, and QNX has dlopen/dlsym in
# libc and no libdl. It goes to $PREFIX/lib/datefudge/datefudge.so, where the
# installed wrapper looks for it.
do_build() {
  rm -rf "$B" && mkdir -p "$B" && cp -a "$S/." "$B/" || return 1
  make -C "$B" VERSION=$VERSION prefix="$PREFIX" datefudge datefudge.1 || return 1
  qcc -V$QCC_TARGET -D_QNX_SOURCE -O2 -Wall -Wextra -fpic -shared -o "$B/datefudge.so" "$B/datefudge.c"
}
# Installed by hand: "make install" depends on datefudge.o, which this build
# never makes, so make would compile and link datefudge.so again -- with the
# build machine's cc (a Linux library in the package).
do_install() {
  install -d "$STAGE$PREFIX/lib/datefudge" "$STAGE$PREFIX/bin" "$STAGE$PREFIX/share/man/man1" || return 1
  install -m 644 "$B/datefudge.so" "$STAGE$PREFIX/lib/datefudge/datefudge.so" &&
  install -m 755 "$B/datefudge" "$STAGE$PREFIX/bin/datefudge" &&
  install -m 644 "$B/datefudge.1" "$STAGE$PREFIX/share/man/man1/datefudge.1"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
