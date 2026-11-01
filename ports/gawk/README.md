# gawk 5.3.2

> GNU awk pattern scanning and text processing language

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/gawk/gawk-5.3.2.tar.xz |
| SHA-256 | `f8c3486509de705192138b00ef2c00bbbdd0e84c30d5c07d23fc73a9dc4cc9cc` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`readline`](../readline/) [`ncurses`](../ncurses/) |
| Run-time dependencies | [`ncurses`](../ncurses/) [`readline`](../readline/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gawk`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gawk` (programs, shared libraries, run-time data) and `gawk-dev` (headers, pkg-config/CMake files, static libraries; needs `gawk`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gawk`](https://openqnx.github.io/oss/package/x86_64/gawk.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gawk.html)) · [`gawk-dev`](https://openqnx.github.io/oss/package/x86_64/gawk-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gawk-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gawk` (dependencies come along); one version: `apk add gawk=<version>` | `apk del gawk` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gawk` (dependencies first) | `oss-get remove gawk` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gawk-dev` the same way, by its own name (`apk add gawk-dev`, `oss-get install gawk-dev`, by hand with its own file and list `gawk-dev.list`); remove it before `gawk`, which it needs.

```sh
# on the QNX target, as root
apk add gawk gawk-dev
apk add qnx-test-gawk && /usr/local/bin/qnx-port-tests/test_gawk    # its on-target test, from tests-8.0-<arch>
apk del gawk gawk-dev qnx-test-gawk
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gawk.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gawk.list
rm var/lib/manual/gawk.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gawk` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (2 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild readline ncurses
```

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/gawk/gawk-5.3.2.tar.xz
mkdir -p dist src/gawk
curl -fL -o dist/$(basename $URL) $URL
echo "f8c3486509de705192138b00ef2c00bbbdd0e84c30d5c07d23fc73a9dc4cc9cc  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/gawk --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/gawk < ports/gawk/patches/0001-aports-fix-qnx.patch
```

**[`0001-aports-fix-qnx.patch`](patches/0001-aports-fix-qnx.patch)** — QNX change from QNX's aports (core/gawk/fix-qnx.patch).

> Taken from qnx-ports/aports core/gawk -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `gawk-5.3.2/awk.h`, `gawk-5.3.2/extension/inplace.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/gawk && cd build/$QNX_ARCH/gawk
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/gawk/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-nls \
    --without-mpfr
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GNU awk. Version as in the QNX Everywhere 8.0.4r1 image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gawk`.

### 5. What it installs

75 files under `/usr/local`, 4 programs, 1 headers.

```
/usr/local/bin/awk
/usr/local/bin/gawk
/usr/local/bin/gawk-5.3.2
/usr/local/bin/gawkbug
```

The complete list is `stage/<arch>/.manifest/gawk` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gawk
ls dists/8.0/*/ | grep '^gawk-'
```

This writes `gawk-<ver>-r0.tar.gz` (run time) and `gawk-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gawk
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gawk-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gawk-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gawk`](../../tests/src/) links gawk the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gawk`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gawk
/usr/local/bin/qnx-port-tests/test_gawk
```

```
== gawk ==
  ok   ...
PASS gawk
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gawk` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gawk`](../../tests/src/) -- a program linked against gawk the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gawk 5.3.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/gawk-5.3.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gawk 5.3.2-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/gawk-5.3.2-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gawk 5.3.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/gawk-5.3.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gawk 5.3.2-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/gawk-5.3.2-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **5.3.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gawk=<version>`), sources in `ported/gawk/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gawk`; port one with `scripts/port-update gawk`.

<details><summary>recipe.sh</summary>

```bash
# GNU awk. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=5.3.2
TARBALL=https://ftp.gnu.org/gnu/gawk/gawk-5.3.2.tar.xz
SHA256=f8c3486509de705192138b00ef2c00bbbdd0e84c30d5c07d23fc73a9dc4cc9cc
DEPS="readline ncurses"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --without-mpfr )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
