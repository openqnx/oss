# bison 3.8.2

> The GNU general-purpose parser generator

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/bison/bison-3.8.2.tar.xz |
| SHA-256 | `9bba0214ccf7f1079c5d59210045227bcf619519840ebfa80cd3849cff5a5bf2` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_bison`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `bison` (programs, shared libraries, run-time data) and `bison-dev` (headers, pkg-config/CMake files, static libraries; needs `bison`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`bison`](https://openqnx.github.io/oss/package/x86_64/bison.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/bison.html)) · [`bison-dev`](https://openqnx.github.io/oss/package/x86_64/bison-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/bison-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add bison` (dependencies come along); one version: `apk add bison=<version>` | `apk del bison` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install bison` (dependencies first) | `oss-get remove bison` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`bison-dev` the same way, by its own name (`apk add bison-dev`, `oss-get install bison-dev`, by hand with its own file and list `bison-dev.list`); remove it before `bison`, which it needs.

```sh
# on the QNX target, as root
apk add bison bison-dev
apk add qnx-test-bison && /usr/local/bin/qnx-port-tests/test_bison    # its on-target test, from tests-8.0-<arch>
apk del bison bison-dev qnx-test-bison
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/bison.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/bison.list
rm var/lib/manual/bison.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild bison` performs exactly the same steps (2–5) in one command.

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
URL=https://ftp.gnu.org/gnu/bison/bison-3.8.2.tar.xz
mkdir -p dist src/bison
curl -fL -o dist/$(basename $URL) $URL
echo "9bba0214ccf7f1079c5d59210045227bcf619519840ebfa80cd3849cff5a5bf2  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/bison --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/bison && cd build/$QNX_ARCH/bison
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
M4=/usr/local/bin/m4 \
ac_cv_path_M4=/usr/local/bin/m4 \
am_cv_func_iconv=no \
ac_cv_libtextstyle=no \
$OLDPWD/src/bison/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-nls \
    --disable-yacc \
    --disable-rpath \
    --without-libtextstyle-prefix \
    --without-libiconv-prefix
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- bison runs m4 to expand its skeletons, at run time: the m4 port's /usr/local/bin/m4 (QNX itself ships none).
- M4: the program bison runs on the target, not a build-machine probe (a cross configure would otherwise record the build machine's m4 path). Neither libiconv nor gettext's libtextstyle (coloured diagnostics): the C locale needs no conversion, and both would otherwise be picked up from staging -- with the staging directory as an rpath (--disable-rpath).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH bison`.

### 5. What it installs

88 files under `/usr/local`, 1 programs.

```
/usr/local/bin/bison
```

The complete list is `stage/<arch>/.manifest/bison` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa bison
ls dists/8.0/*/ | grep '^bison-'
```

This writes `bison-<ver>-r0.tar.gz` (run time) and `bison-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add bison
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/bison-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/bison-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_bison`](../../tests/src/) links bison the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-bison`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-bison
/usr/local/bin/qnx-port-tests/test_bison
```

```
== bison ==
  ok   ...
PASS bison
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_bison` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_bison`](../../tests/src/) -- a program linked against bison the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | bison 3.8.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/bison-3.8.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | bison 3.8.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/bison-3.8.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.8.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add bison=<version>`), sources in `ported/bison/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates bison`; port one with `scripts/port-update bison`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.8.2
TARBALL=https://ftp.gnu.org/gnu/bison/bison-3.8.2.tar.xz
SHA256=9bba0214ccf7f1079c5d59210045227bcf619519840ebfa80cd3849cff5a5bf2
BUILDSYS=autotools
# bison runs m4 to expand its skeletons, at run time: the m4 port's
# /usr/local/bin/m4 (QNX itself ships none).
RUNTIME_DEPS="m4"
# M4: the program bison runs on the target, not a build-machine probe (a
# cross configure would otherwise record the build machine's m4 path).
# Neither libiconv nor gettext's libtextstyle (coloured diagnostics): the
# C locale needs no conversion, and both would otherwise be picked up from
# staging -- with the staging directory as an rpath (--disable-rpath).
CONF_ENV=( M4=/usr/local/bin/m4 ac_cv_path_M4=/usr/local/bin/m4 am_cv_func_iconv=no ac_cv_libtextstyle=no )
CONF_ARGS_EXTRA=( --disable-nls --disable-yacc --disable-rpath --without-libtextstyle-prefix --without-libiconv-prefix )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
