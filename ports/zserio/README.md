# zserio 2.20.0

> C++ runtime library of zserio, a serialization framework for binary data

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/ndsev/zserio.git (tag `v2.20.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_zserio`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `zserio-dev` -- header-only: there is no run-time package, the -dev package is the whole component (headers, pkg-config/CMake files).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`zserio-dev`](https://openqnx.github.io/oss/package/x86_64/zserio-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/zserio-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add zserio-dev` (dependencies come along); one version: `apk add zserio-dev=<version>` | `apk del zserio-dev` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install zserio-dev` (dependencies first) | `oss-get remove zserio-dev` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-zserio && /usr/local/bin/qnx-port-tests/test_zserio    # its on-target test, from tests-8.0-<arch>-m-z
apk del zserio-dev qnx-test-zserio
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/zserio-dev.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/zserio-dev.list
rm var/lib/manual/zserio-dev.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild zserio` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2.20.0 --recurse-submodules --shallow-submodules \
    https://github.com/ndsev/zserio.git src/zserio
```

The project lives in the `compiler/extensions/cpp/runtime/src/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/zserio/compiler/extensions/cpp/runtime/src; B=build/$QNX_ARCH/zserio; mkdir -p $B
```

```bash
do_install() {
  local inc="$STAGE$PREFIX/include" f
  mkdir -p "$inc" "$STAGE$PREFIX/lib" || return 1
  ( cd "$S" && find zserio -name '*.h' ) | while read -r f; do
    install -Dm644 "$S/$f" "$inc/$f" || exit 1; done || return 1
  install -m644 "$B/libZserioCppRuntime.a" "$STAGE$PREFIX/lib/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- zserio: serialization framework for binary data (a schema language, a code generator and per-language runtimes). This is the C++ runtime library: bit stream reader/writer, bit buffers, arrays and packing contexts, varints, JSON encoding/decoding, type info and reflection, the pmr allocators, and the interfaces generated services, pub/sub and SQLite databases implement. The code generator is a Java program that runs on the build machine (zserio.jar, from the release or `pip install zserio`); it is not part of this package, and code generated with the same zserio release compiles against these headers.
- Upstream builds the runtime as a static library and has no install rules for it (its own "install" copies the sources). The archive is built with -fPIC so that it can go into shared libraries of generated code.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH zserio`.

### 5. What it installs

82 files under `/usr/local`, 81 headers.

The complete list is `stage/<arch>/.manifest/zserio` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa zserio
ls dists/8.0/*/ | grep '^zserio-'
```

This writes `zserio-<ver>-r0.tar.gz` (run time) and `zserio-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add zserio
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/zserio-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/zserio-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_zserio`](../../tests/src/) links zserio the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-zserio`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-zserio
/usr/local/bin/qnx-port-tests/test_zserio
```

```
== zserio ==
  ok   ...
PASS zserio
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_zserio` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_zserio`](../../tests/src/) -- a program linked against zserio the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - 24 bits written
   - bit fields packed MSB first: BF A1 23
   - bit fields read back
   - signed 4-bit fields: -1, -8, 7
   - 64-bit and signed 48-bit fields
   - varuint16 1, 300; varint16 -1; varsize 127: 01 81 2C 81 7F
   - bitSizeOfVarUInt16/VarInt16
   - bitSizeOf for varuint64 (8 bytes at 2^56) and varsize
   - varints read back
   - varuint64 (56 bits), varint INT64_MIN, varint64/varuint32/varint32 extremes
   - varuint16 out of range throws CppRuntimeException
   - string: varsize length then UTF-8 bytes
   - float16 1.0 is 3C00
   - half-precision conversions (-2.0, max 65504)
   - alignTo(8)
   - string and floats read back
   - bools read back
   - bytes read back
   - UTF-8 string read back, position at the end
   - reading past the end throws CppRuntimeException
   - setBitPosition and read again
   - BitBuffer of 10 bits
   - BitBuffer equality ignores bits past the size
   - bit buffer embedded at an odd bit offset
   - writeBufferToFile / readBufferFromFile
   - ... and 1 more

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> zserio`.

## Versions

This recipe builds **2.20.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add zserio=<version>`), sources in `ported/zserio/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates zserio`; port one with `scripts/port-update zserio`.

<details><summary>recipe.sh</summary>

```bash
# zserio: serialization framework for binary data (a schema language, a code
# generator and per-language runtimes). This is the C++ runtime library:
# bit stream reader/writer, bit buffers, arrays and packing contexts,
# varints, JSON encoding/decoding, type info and reflection, the pmr
# allocators, and the interfaces generated services, pub/sub and SQLite
# databases implement.
# The code generator is a Java program that runs on the build machine
# (zserio.jar, from the release or `pip install zserio`); it is not part of
# this package, and code generated with the same zserio release compiles
# against these headers.
VERSION=2.20.0; GIT=https://github.com/ndsev/zserio.git; REF=v2.20.0
SUBDIR=compiler/extensions/cpp/runtime/src
# Upstream builds the runtime as a static library and has no install rules
# for it (its own "install" copies the sources). The archive is built with
# -fPIC so that it can go into shared libraries of generated code.
CMAKE_ARGS_EXTRA=( -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
do_install() {
  local inc="$STAGE$PREFIX/include" f
  mkdir -p "$inc" "$STAGE$PREFIX/lib" || return 1
  ( cd "$S" && find zserio -name '*.h' ) | while read -r f; do
    install -Dm644 "$S/$f" "$inc/$f" || exit 1; done || return 1
  install -m644 "$B/libZserioCppRuntime.a" "$STAGE$PREFIX/lib/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
