# swig 4.3.1

> Simplified Wrapper and Interface Generator: C/C++ bindings for Python, Perl, Lua, Tcl and more

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/project/swig/swig/swig-4.3.1/swig-4.3.1.tar.gz |
| SHA-256 | `44fc829f70f1e17d635a2b4d69acab38896699ecc24aa023e516e0eabbec61b8` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`pcre2`](../pcre2/) |
| Run-time dependencies | [`pcre2`](../pcre2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_swig`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `swig`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`swig`](https://openqnx.github.io/oss/package/x86_64/swig.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/swig.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add swig` (dependencies come along); one version: `apk add swig=<version>` | `apk del swig` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install swig` (dependencies first) | `oss-get remove swig` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-swig && /usr/local/bin/qnx-port-tests/test_swig    # its on-target test, from tests-8.0-<arch>-m-z
apk del swig qnx-test-swig
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/swig.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/swig.list
rm var/lib/manual/swig.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild swig` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (3 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild pcre2
```

<details><summary>Full build chain (3)</summary>

[`bzip2`](../bzip2/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://downloads.sourceforge.net/project/swig/swig/swig-4.3.1/swig-4.3.1.tar.gz
mkdir -p dist src/swig
curl -fL -o dist/$(basename $URL) $URL
echo "44fc829f70f1e17d635a2b4d69acab38896699ecc24aa023e516e0eabbec61b8  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/swig --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/swig && cd build/$QNX_ARCH/swig
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
PCRE2_CFLAGS=-D_QNX_SOURCE \
PCRE2_LIBS=-lpcre2-8 \
$OLDPWD/src/swig/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --without-alllang \
    --with-pcre \
    --disable-ccache
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- SWIG, the Simplified Wrapper and Interface Generator: generates bindings for C and C++ libraries (Python, Perl, Tcl, Lua, Java, ... -- every target language module is built into the one swig program). Version as in QNX's own package list.
- pcre2: %rename("%(regex:...)") and the other regex-based name matching.
- Autotools, not the CMake build: the release tarball's configure uses its pre-generated parser (CMakeLists.txt insists on bison).
- --without-alllang: the language probes only choose which test suites run (on the build machine); the swig program has every language module. PCRE2: the staged library, named directly -- otherwise configure runs the build machine's pcre2-config. (Its headers are on the staging include path; PCRE2_CFLAGS only has to be non-empty.) --disable-ccache: no ccache-swig, SWIG's old private ccache copy (the ccache port is the real one).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH swig`.

### 5. What it installs

837 files under `/usr/local`, 1 programs.

```
/usr/local/bin/swig
```

The complete list is `stage/<arch>/.manifest/swig` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa swig
ls dists/8.0/*/ | grep '^swig-'
```

This writes `swig-<ver>-r0.tar.gz` (run time) and `swig-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add swig
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/swig-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/swig-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_swig`](../../tests/src/) links swig the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-swig`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-swig
/usr/local/bin/qnx-port-tests/test_swig
```

```
== swig ==
  ok   ...
PASS swig
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_swig` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_swig`](../../tests/src/) -- a program linked against swig the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | swig 4.3.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/swig-4.3.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | swig 4.3.1-r0 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/swig-4.3.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.3.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add swig=<version>`), sources in `ported/swig/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates swig`; port one with `scripts/port-update swig`.

<details><summary>recipe.sh</summary>

```bash
# SWIG, the Simplified Wrapper and Interface Generator: generates bindings
# for C and C++ libraries (Python, Perl, Tcl, Lua, Java, ... -- every target
# language module is built into the one swig program). Version as in QNX's
# own package list.
VERSION=4.3.1
TARBALL=https://downloads.sourceforge.net/project/swig/swig/swig-4.3.1/swig-4.3.1.tar.gz
SHA256=44fc829f70f1e17d635a2b4d69acab38896699ecc24aa023e516e0eabbec61b8
# pcre2: %rename("%(regex:...)") and the other regex-based name matching.
DEPS="pcre2"
# Autotools, not the CMake build: the release tarball's configure uses its
# pre-generated parser (CMakeLists.txt insists on bison).
BUILDSYS=autotools
# --without-alllang: the language probes only choose which test suites run
# (on the build machine); the swig program has every language module.
# PCRE2: the staged library, named directly -- otherwise configure runs the
# build machine's pcre2-config. (Its headers are on the staging include path;
# PCRE2_CFLAGS only has to be non-empty.)
# --disable-ccache: no ccache-swig, SWIG's old private ccache copy (the ccache
# port is the real one).
CONF_ENV=( PCRE2_CFLAGS=-D_QNX_SOURCE PCRE2_LIBS=-lpcre2-8 )
CONF_ARGS_EXTRA=( --without-alllang --with-pcre --disable-ccache )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
