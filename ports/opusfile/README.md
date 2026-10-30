# opusfile 0.12

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://downloads.xiph.org/releases/opus/opusfile-0.12.tar.gz |
| SHA-256 | `118d8601c12dd6a44f52423e68ca9083cc9f2bfe72da7a8c1acb22a80ae3550b` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`opus`](../opus/) [`libogg`](../libogg/) [`openssl`](../openssl/) |
| Run-time dependencies | [`libogg`](../libogg/) [`openssl`](../openssl/) [`opus`](../opus/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_opusfile`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add opusfile
apk add opusfile-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-opusfile && /usr/local/bin/qnx-port-tests/test_opusfile    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install opusfile`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild opusfile` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild opus libogg openssl
```

### 2. Get the upstream source

```bash
URL=https://downloads.xiph.org/releases/opus/opusfile-0.12.tar.gz
mkdir -p dist src/opusfile
curl -fL -o dist/$(basename $URL) $URL
echo "118d8601c12dd6a44f52423e68ca9083cc9f2bfe72da7a8c1acb22a80ae3550b  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/opusfile --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/opusfile < ports/opusfile/patches/0001-qnx-c99-headers.patch
```

**[`0001-qnx-c99-headers.patch`](patches/0001-qnx-c99-headers.patch)** — Compile as C99 on QNX: its C headers are not C89

> opusfile compiles with -std=c89 -pedantic. QNX's C library headers are C99
> -- stdlib.h declares atexit() "static inline" -- and do not compile as C89
> ("expected ';' before 'int'"). On QNX add -std=gnu99 after it (the last -std
> wins); the warnings it asks for stay.

Touches: `configure.ac` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/opusfile && cd build/$QNX_ARCH/opusfile
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/opusfile/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-examples \
    --disable-doc
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- opusfile: decoding and seeking in .opus files (Ogg Opus), from files or HTTP(S) streams.
- The examples are not installed; seeking_example.c also redefines a stdlib.h keyword QNX's header uses.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH opusfile`.

### 5. What it installs

10 files under `/usr/local`: 4 shared-library entries, 1 headers.

```
/usr/local/lib/libopusfile.so
/usr/local/lib/libopusfile.so.4
/usr/local/lib/libopusurl.so
/usr/local/lib/libopusurl.so.4
```

The complete list is `stage/<arch>/.manifest/opusfile` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa opusfile
ls dists/8.0/*/ | grep '^opusfile-'
```

This writes `opusfile-<ver>-r0.tar.gz` (run time) and `opusfile-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add opusfile
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/opusfile-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/opusfile-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_opusfile`](../../tests/src/) links opusfile the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-opusfile`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-opusfile
/usr/local/bin/qnx-port-tests/test_opusfile
```

```
== opusfile ==
  ok   ...
PASS opusfile
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_opusfile` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_opusfile`](../../tests/src/) -- a program linked against opusfile the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - Ogg page made
   - op_test recognises the Opus stream
   - OpusHead fields
   - opus_tags_parse
   - vendor and one comment
   - TITLE=Test1

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | opusfile 0.12-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | opusfile 0.12-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **0.12**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add opusfile=<version>`), sources in `ported/opusfile/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates opusfile`; port one with `scripts/port-update opusfile`.

<details><summary>recipe.sh</summary>

```bash
# opusfile: decoding and seeking in .opus files (Ogg Opus), from files or HTTP(S) streams.
VERSION=0.12
TARBALL=https://downloads.xiph.org/releases/opus/opusfile-0.12.tar.gz
SHA256=118d8601c12dd6a44f52423e68ca9083cc9f2bfe72da7a8c1acb22a80ae3550b
DEPS="opus libogg openssl"
BUILDSYS=autotools
# The examples are not installed; seeking_example.c also redefines a stdlib.h
# keyword QNX's header uses.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-examples --disable-doc )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
