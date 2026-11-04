# libfyaml 0.9.6

> YAML 1.2 parser and emitter library in C, with the fy-tool program

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/pantoniou/libfyaml/releases/download/v0.9.6/libfyaml-0.9.6.tar.gz |
| SHA-256 | `a59cc3331e2eb903ec36933ad52a45888041cac31e44f553a00511131242c483` |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libfyaml`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libfyaml` (programs, shared libraries, run-time data) and `libfyaml-dev` (headers, pkg-config/CMake files, static libraries; needs `libfyaml`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libfyaml`](https://openqnx.github.io/oss/package/x86_64/libfyaml.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libfyaml.html)) · [`libfyaml-dev`](https://openqnx.github.io/oss/package/x86_64/libfyaml-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libfyaml-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libfyaml` (dependencies come along); one version: `apk add libfyaml=<version>` | `apk del libfyaml` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libfyaml` (dependencies first) | `oss-get remove libfyaml` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libfyaml-dev` the same way, by its own name (`apk add libfyaml-dev`, `oss-get install libfyaml-dev`, by hand with its own file and list `libfyaml-dev.list`); remove it before `libfyaml`, which it needs.

```sh
# on the QNX target, as root
apk add libfyaml libfyaml-dev
apk add qnx-test-libfyaml && /usr/local/bin/qnx-port-tests/test_libfyaml    # its on-target test, from tests-8.0-<arch>
apk del libfyaml libfyaml-dev qnx-test-libfyaml
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libfyaml.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libfyaml.list
rm var/lib/manual/libfyaml.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libfyaml` performs exactly the same steps (2–5) in one command.

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
URL=https://github.com/pantoniou/libfyaml/releases/download/v0.9.6/libfyaml-0.9.6.tar.gz
mkdir -p dist src/libfyaml
curl -fL -o dist/$(basename $URL) $URL
echo "a59cc3331e2eb903ec36933ad52a45888041cac31e44f553a00511131242c483  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libfyaml --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libfyaml < ports/libfyaml/patches/0001-qnx-endian.patch
patch -p1 -d src/libfyaml < ports/libfyaml/patches/0002-memstream-tmpfile.patch
```

**[`0001-qnx-endian.patch`](patches/0001-qnx-endian.patch)** — Take the byte order macros from <sys/param.h> on QNX.

> The public header libfyaml-endian.h knows where each OS keeps its byte
> order macros and stops with "unsupported platform" elsewhere. QNX has
> them (BYTE_ORDER, LITTLE_ENDIAN, BIG_ENDIAN) in <sys/param.h>; the header
> already maps those spellings to __BYTE_ORDER and friends. (qnx-ports'
> libfyaml 0.9.5 has the same fix in the older internal fy-endian.h.)

Touches: `include/libfyaml/libfyaml-endian.h` · Upstream status: Pending

**[`0002-memstream-tmpfile.patch`](patches/0002-memstream-tmpfile.patch)** — Use the tmpfile() memory stream on QNX, which has no open_memstream().

> QNX has no open_memstream(), and none of funopen()/fopencookie() that
> libfyaml's macOS replacement is built on, so libfyaml.so had an undefined
> reference to open_memstream. fy_memstream_open/close (the only users) already
> have a tmpfile() variant for Windows: the stream is a temporary file, read
> back into a malloc'ed buffer at close. Use it on QNX too.

Touches: `src/util/fy-utils.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libfyaml -B build/$QNX_ARCH/libfyaml -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DSPHINX_EXECUTABLE=OFF \
      -DENABLE_LIBCLANG=OFF \
      -DBUILD_TESTING=OFF \
      -DENABLE_NETWORK=OFF
cmake --build build/$QNX_ARCH/libfyaml -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libfyaml
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libfyaml: a complete YAML 1.2 (and 1.1) parser and emitter in C, with a document tree API, path expressions and the fy-tool program (fy-dump, fy-filter, fy-join, fy-testsuite are links to it).
- The man pages would be built by Sphinx at install time, with an extension (linuxdoc) the build machine does not have: no Sphinx. libclang (reflection) is off, the upstream default. Tests are not installed.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libfyaml`.

### 5. What it installs

36 files under `/usr/local`: 3 shared-library entries, 7 programs, 14 headers.

```
/usr/local/lib/libfyaml.so
/usr/local/lib/libfyaml.so.0
/usr/local/lib/libfyaml.so.0.9.6
/usr/local/bin/fy-compose
/usr/local/bin/fy-dump
/usr/local/bin/fy-filter
/usr/local/bin/fy-join
/usr/local/bin/fy-testsuite
/usr/local/bin/fy-tool
/usr/local/bin/fy-ypath
```

The complete list is `stage/<arch>/.manifest/libfyaml` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libfyaml
ls dists/8.0/*/ | grep '^libfyaml-'
```

This writes `libfyaml-<ver>-r0.tar.gz` (run time) and `libfyaml-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libfyaml
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libfyaml-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libfyaml-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libfyaml`](../../tests/src/) links libfyaml the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libfyaml`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libfyaml
/usr/local/bin/qnx-port-tests/test_libfyaml
```

```
== libfyaml ==
  ok   ...
PASS libfyaml
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libfyaml` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libfyaml`](../../tests/src/) -- a program linked against libfyaml the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - document parsed
   - /name = qnx-oss
   - /arches/1 = aarch64le
   - /arches has 2 items
   - block literal keeps its newlines
   - /service/host = localhost
   - anchored /defaults/timeout = 30
   - /count carries the !!int tag
   - fy_document_scanf: 30, qnx-oss, 42
   - emitted text keeps keys and the alias
   - round trip: re-parsed tree equals the original
   - one-line flow emission
   - one-line JSON emission
   - document built from code emits {list: [1, 2], key: value}
   - unterminated flow sequence is an error

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> libfyaml`.

## Versions

This recipe builds **0.9.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libfyaml=<version>`), sources in `ported/libfyaml/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libfyaml`; port one with `scripts/port-update libfyaml`.

<details><summary>recipe.sh</summary>

```bash
# libfyaml: a complete YAML 1.2 (and 1.1) parser and emitter in C, with a
# document tree API, path expressions and the fy-tool program (fy-dump,
# fy-filter, fy-join, fy-testsuite are links to it).
VERSION=0.9.6
TARBALL=https://github.com/pantoniou/libfyaml/releases/download/v0.9.6/libfyaml-0.9.6.tar.gz
SHA256=a59cc3331e2eb903ec36933ad52a45888041cac31e44f553a00511131242c483
BUILDSYS=cmake
# The man pages would be built by Sphinx at install time, with an extension
# (linuxdoc) the build machine does not have: no Sphinx. libclang
# (reflection) is off, the upstream default. Tests are not installed.
CMAKE_ARGS_EXTRA=( -DSPHINX_EXECUTABLE=OFF -DENABLE_LIBCLANG=OFF -DBUILD_TESTING=OFF -DENABLE_NETWORK=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
