# riegeli 0_git20231212

> File format for storing a sequence of records, typically protocol buffers

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/google/riegeli.git |
| Build system | custom (see recipe) |
| QNX patches | 3 |
| Build dependencies | [`abseil`](../abseil/) [`protobuf`](../protobuf/) [`brotli`](../brotli/) [`zstd`](../zstd/) [`snappy`](../snappy/) [`xz`](../xz/) [`zlib`](../zlib/) [`bzip2`](../bzip2/) [`lz4`](../lz4/) [`highwayhash`](../highwayhash/) |
| Run-time dependencies | [`abseil`](../abseil/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`highwayhash`](../highwayhash/) [`lz4`](../lz4/) [`protobuf`](../protobuf/) [`snappy`](../snappy/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_riegeli`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `riegeli` (programs, shared libraries, run-time data) and `riegeli-dev` (headers, pkg-config/CMake files, static libraries; needs `riegeli`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`riegeli`](https://openqnx.github.io/oss/package/x86_64/riegeli.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/riegeli.html)) · [`riegeli-dev`](https://openqnx.github.io/oss/package/x86_64/riegeli-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/riegeli-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add riegeli` (dependencies come along); one version: `apk add riegeli=<version>` | `apk del riegeli` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install riegeli` (dependencies first) | `oss-get remove riegeli` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`riegeli-dev` the same way, by its own name (`apk add riegeli-dev`, `oss-get install riegeli-dev`, by hand with its own file and list `riegeli-dev.list`); remove it before `riegeli`, which it needs.

```sh
# on the QNX target, as root
apk add riegeli riegeli-dev
apk add qnx-test-riegeli && /usr/local/bin/qnx-port-tests/test_riegeli    # its on-target test, from tests-8.0-<arch>-m-z
apk del riegeli riegeli-dev qnx-test-riegeli
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/riegeli.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/riegeli.list
rm var/lib/manual/riegeli.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild riegeli` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (14 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild abseil protobuf brotli zstd snappy xz zlib bzip2 lz4 highwayhash
```

<details><summary>Full build chain (14)</summary>

[`abseil`](../abseil/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`gettext`](../gettext/) [`highwayhash`](../highwayhash/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`lz4`](../lz4/) [`ncurses`](../ncurses/) [`protobuf`](../protobuf/) [`snappy`](../snappy/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/google/riegeli.git src/riegeli
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/riegeli < ports/riegeli/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/riegeli < ports/riegeli/patches/0002-installed-header-paths.patch
patch -p1 -d src/riegeli < ports/riegeli/patches/0003-protobuf-error-collector.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — riegeli on QNX, from QNX's port (github.com/qnx-ports/riegeli, tag

> qnx-e54e4300-r0, commit 8eebcced): the CRC-32C digester computes with
> abseil's absl::ExtendCrc32c() instead of the standalone crc32c library,
> which is not ported to QNX; abseil is a dependency already, and QNX checked
> that the result matches absl::ComputeCrc32c() bit for bit.

Touches: `riegeli/digests/crc32c_digester.h` · Upstream status: Inappropriate [QNX's port, kept as QNX published it]

**[`0002-installed-header-paths.patch`](patches/0002-installed-header-paths.patch)** — Include zlib's and LZ4's headers by their installed names, not Bazel paths.

> Two sources name a dependency's header by the path it has in riegeli's
> Bazel workspace rather than where the library installs it: the Adler-32
> and CRC-32 digesters include zlib's "src/zconf.h" (zlib_reader.cc and
> zlib_writer.cc already include "zconf.h"), and lz4_reader.h, a public
> header, includes "third_party/lz4/lib/lz4.h". Outside Bazel (this port
> builds with CMake, against the installed zlib and lz4) they are "zconf.h"
> and "lz4.h".

Touches: `riegeli/digests/adler32_digester.cc`, `riegeli/digests/crc32_digester.cc`, `riegeli/lz4/lz4_reader.h` · Upstream status: Pending

**[`0003-protobuf-error-collector.patch`](patches/0003-protobuf-error-collector.patch)** — Override protobuf's RecordError(), the error-collector hook since protobuf 22.

> protobuf 22 renamed the error-collector hooks and took string_view
> arguments: io::ErrorCollector::AddError(int, ColumnNumber, const string&)
> became RecordError(int, ColumnNumber, absl::string_view), and
> DescriptorPool::ErrorCollector::AddError(...) RecordError(...); the old
> names were removed in protobuf 26. This tree's protobuf is 29.3, so the
> two collectors riegeli defines (text parsing, records metadata) override
> the new names. Upstream riegeli made the same change later.

Touches: `riegeli/messages/text_parse.cc`, `riegeli/messages/text_parse.h`, `riegeli/records/record_reader.cc` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/riegeli; B=build/$QNX_ARCH/riegeli; mkdir -p $B
```

```bash
do_configure() {
  cmake -S "$RECIPE_DIR/riegeli/files" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DRIEGELI_SOURCE_DIR="$S" -DPROTOC="$(_host_protoc)" \
    -Dabsl_DIR="$STAGE$PREFIX/lib/cmake/absl" -Dprotobuf_DIR="$STAGE$PREFIX/lib/cmake/protobuf"
}
```

```bash
do_build() { cmake --build "$B" -j"$(nproc)"; }
```

```bash
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- riegeli: Google's file format for sequences of records (protocol buffer messages or plain strings) with compression, checksums and recovery from damage -- libriegeli (its readers and writers, records, chunk encodings, Brotli/Zstd/Snappy/xz/zlib/bzip2/LZ4 compression) and describe_riegeli_file. With QNX's change (patches/0001, from github.com/qnx-ports/riegeli, tag qnx-e54e4300-r0; upstream commit e54e4300 of 2023-12-12 is what QNX's port is based on, and it builds against this tree's abseil 20240722).
- Upstream builds only with Bazel. files/CMakeLists.txt builds the library from the same sources (all of riegeli/ except the TensorFlow ops). The protobuf code is generated with the host protoc the protobuf recipe built (29.3, the version its generated code must match). Left out: ReaderCFile()/ WriterCFile() (a Reader/Writer as a stdio FILE*), made with glibc's fopencookie(), which QNX's C library does not have. snappy is built without RTTI, so riegeli's snappy stream adapters are too. patches/0002: zlib's and LZ4's headers by their installed names (riegeli named them by Bazel paths); patches/0003: protobuf 29's error-collector interface (RecordError).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH riegeli`.

### 5. What it installs

186 files under `/usr/local`: 3 shared-library entries, 1 programs, 182 headers.

```
/usr/local/lib/libriegeli.so
/usr/local/lib/libriegeli.so.0
/usr/local/lib/libriegeli.so.0.0.0
/usr/local/bin/describe_riegeli_file
```

The complete list is `stage/<arch>/.manifest/riegeli` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa riegeli
ls dists/8.0/*/ | grep '^riegeli-'
```

This writes `riegeli-<ver>-r0.tar.gz` (run time) and `riegeli-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add riegeli
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/riegeli-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/riegeli-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_riegeli`](../../tests/src/) links riegeli the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-riegeli`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-riegeli
/usr/local/bin/qnx-port-tests/test_riegeli
```

```
== riegeli ==
  ok   ...
PASS riegeli
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_riegeli` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_riegeli`](../../tests/src/) -- a program linked against riegeli the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - CRC-32C(\"123456789\") = 0xe3069283
   - 100 proto records written, transposed
   - file metadata read back
   - 100 proto records parsed back unchanged
   - file written with FdWriter (small chunks)
   - seek to the position of record 1234 and read it
   - then record 1235 follows
   - reader closed cleanly
   - damaged file: reading without recovery fails
   - with recovery: the damaged region is skipped, the rest read

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | riegeli 0_git20231212-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/riegeli-0_git20231212-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | riegeli 0_git20231212-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/riegeli-0_git20231212-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0_git20231212**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add riegeli=<version>`), sources in `ported/riegeli/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates riegeli`; port one with `scripts/port-update riegeli`.

<details><summary>recipe.sh</summary>

```bash
# riegeli: Google's file format for sequences of records (protocol buffer
# messages or plain strings) with compression, checksums and recovery from
# damage -- libriegeli (its readers and writers, records, chunk encodings,
# Brotli/Zstd/Snappy/xz/zlib/bzip2/LZ4 compression) and describe_riegeli_file.
# With QNX's change (patches/0001, from github.com/qnx-ports/riegeli, tag
# qnx-e54e4300-r0; upstream commit e54e4300 of 2023-12-12 is what QNX's port
# is based on, and it builds against this tree's abseil 20240722).
VERSION=0_git20231212; GIT=https://github.com/google/riegeli.git; COMMIT=e54e4300f61cc113e5da816952d20ae070fe7219
DEPS="abseil protobuf brotli zstd snappy xz zlib bzip2 lz4 highwayhash"
BUILDSYS=custom
# Upstream builds only with Bazel. files/CMakeLists.txt builds the library
# from the same sources (all of riegeli/ except the TensorFlow ops). The
# protobuf code is generated with the host protoc the protobuf recipe built
# (29.3, the version its generated code must match). Left out: ReaderCFile()/
# WriterCFile() (a Reader/Writer as a stdio FILE*), made with glibc's
# fopencookie(), which QNX's C library does not have. snappy is built without
# RTTI, so riegeli's snappy stream adapters are too. patches/0002: zlib's and
# LZ4's headers by their installed names (riegeli named them by Bazel paths);
# patches/0003: protobuf 29's error-collector interface (RecordError).
_host_protoc() { find "$BUILD_DIR/host/protobuf" -maxdepth 1 -name 'protoc*' -type f -perm -u+x | head -1; }
do_configure() {
  cmake -S "$RECIPE_DIR/riegeli/files" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DRIEGELI_SOURCE_DIR="$S" -DPROTOC="$(_host_protoc)" \
    -Dabsl_DIR="$STAGE$PREFIX/lib/cmake/absl" -Dprotobuf_DIR="$STAGE$PREFIX/lib/cmake/protobuf"
}
do_build() { cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
