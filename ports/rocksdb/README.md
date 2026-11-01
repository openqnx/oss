# rocksdb 11.8.1

> Embeddable, persistent key-value store for fast storage

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/facebook/rocksdb.git (tag `v11.8.1`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`lz4`](../lz4/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Run-time dependencies | [`lz4`](../lz4/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_rocksdb`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `rocksdb` (programs, shared libraries, run-time data) and `rocksdb-dev` (headers, pkg-config/CMake files, static libraries; needs `rocksdb`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`rocksdb`](https://openqnx.github.io/oss/package/x86_64/rocksdb.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/rocksdb.html)) · [`rocksdb-dev`](https://openqnx.github.io/oss/package/x86_64/rocksdb-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/rocksdb-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add rocksdb` (dependencies come along); one version: `apk add rocksdb=<version>` | `apk del rocksdb` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install rocksdb` (dependencies first) | `oss-get remove rocksdb` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`rocksdb-dev` the same way, by its own name (`apk add rocksdb-dev`, `oss-get install rocksdb-dev`, by hand with its own file and list `rocksdb-dev.list`); remove it before `rocksdb`, which it needs.

```sh
# on the QNX target, as root
apk add rocksdb rocksdb-dev
apk add qnx-test-rocksdb && /usr/local/bin/qnx-port-tests/test_rocksdb    # its on-target test, from tests-8.0-<arch>-m-z
apk del rocksdb rocksdb-dev qnx-test-rocksdb
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/rocksdb.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/rocksdb.list
rm var/lib/manual/rocksdb.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild rocksdb` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild lz4 zlib zstd
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v11.8.1 --recurse-submodules --shallow-submodules \
    https://github.com/facebook/rocksdb.git src/rocksdb
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/rocksdb < ports/rocksdb/patches/0001-qnx-platform.patch
```

**[`0001-qnx-platform.patch`](patches/0001-qnx-platform.patch)** — Support QNX as a POSIX platform (OS_QNX).

> RocksDB selects its platform code by OS_* macros derived from
> CMAKE_SYSTEM_NAME, and QNX is not one of them, so port_posix.h fell through
> to <endian.h>, which QNX does not have. This adds OS_QNX and, under it:
>  - the byte order from the compiler's __BYTE_ORDER__ (port/port_posix.h);
>  - fread/fwrite/fflush for the *_unlocked variants, which QNX's libc does
>    not provide (as on the BSDs, macOS, Solaris and AIX);
>  - no O_DIRECT in env/fs_posix.cc: QNX has none (as on macOS, OpenBSD and
>    Solaris); direct I/O requests then open the file normally;
>  - GetChildren() without struct dirent's d_type, which QNX lacks: "." and
>    ".." are recognised by name, and only the do_not_recurse case asks
>    stat() whether an entry is a directory.

Touches: `CMakeLists.txt`, `env/fs_posix.cc`, `port/port_posix.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/rocksdb; B=build/$QNX_ARCH/rocksdb; mkdir -p $B
```

```bash
do_install() {
  DESTDIR="$STAGE" cmake --install "$B"
  install -Dm755 "$B/tools/ldb" "$STAGE$PREFIX/bin/ldb"
  install -Dm755 "$B/tools/sst_dump" "$STAGE$PREFIX/bin/sst_dump"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- RocksDB: embeddable persistent key-value store (LSM tree) for fast storage, with column families, transactions, snapshots, iterators, compaction and backups; plus the ldb and sst_dump tools.
- Compression: LZ4, zlib and ZSTD. Not snappy: the snappy component is built as upstream builds it, without RTTI, and RocksDB (built with RTTI, which its users need to derive from its classes) subclasses snappy::Sink, whose typeinfo then does not exist ("undefined reference to typeinfo for snappy::Sink").
- The core tools (ldb, sst_dump) are built but have no install rule.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH rocksdb`.

### 5. What it installs

139 files under `/usr/local`: 3 shared-library entries, 2 programs, 119 headers.

```
/usr/local/lib/librocksdb.so
/usr/local/lib/librocksdb.so.11
/usr/local/lib/librocksdb.so.11.8.1
/usr/local/bin/ldb
/usr/local/bin/sst_dump
```

The complete list is `stage/<arch>/.manifest/rocksdb` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa rocksdb
ls dists/8.0/*/ | grep '^rocksdb-'
```

This writes `rocksdb-<ver>-r0.tar.gz` (run time) and `rocksdb-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add rocksdb
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/rocksdb-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/rocksdb-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_rocksdb`](../../tests/src/) links rocksdb the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-rocksdb`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-rocksdb
/usr/local/bin/qnx-port-tests/test_rocksdb
```

```
== rocksdb ==
  ok   ...
PASS rocksdb
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_rocksdb` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_rocksdb`](../../tests/src/) -- a program linked against rocksdb the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - DB::Open with create_if_missing
   - Put/Get
   - Delete -> NotFound
   - WriteBatch of 5000 puts and a delete
   - batch applied atomically
   - snapshot keeps the old value
   - iteration: 4999 keys in order
   - Seek lands on the next key
   - SeekForPrev skips the deleted key
   - CreateColumnFamily meta
   - column families are separate
   - merge operator: 1+2+...+10 = 55
   - Flush
   - CompactRange over the whole key space
   - compacted to one level: 4000 keys + the merged counter, tombstones dropped
   - first key after the deletions is key01000
   - checkpoint created
   - Close
   - reopen with both column families
   - data, deletions, merges and the column family survived
   - checkpoint opens read-only with the data
   - OptimisticTransactionDB::Open
   - conflicting optimistic transaction fails to commit (Busy)
   - a clean transaction commits both keys
   - ldb get reads a value
   - ... and 3 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | rocksdb 11.8.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/rocksdb-11.8.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | rocksdb 11.8.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/rocksdb-11.8.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **11.8.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add rocksdb=<version>`), sources in `ported/rocksdb/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates rocksdb`; port one with `scripts/port-update rocksdb`.

<details><summary>recipe.sh</summary>

```bash
# RocksDB: embeddable persistent key-value store (LSM tree) for fast storage,
# with column families, transactions, snapshots, iterators, compaction and
# backups; plus the ldb and sst_dump tools.
VERSION=11.8.1; GIT=https://github.com/facebook/rocksdb.git; REF=v11.8.1
# Compression: LZ4, zlib and ZSTD. Not snappy: the snappy component is built
# as upstream builds it, without RTTI, and RocksDB (built with RTTI, which
# its users need to derive from its classes) subclasses snappy::Sink, whose
# typeinfo then does not exist ("undefined reference to typeinfo for
# snappy::Sink").
DEPS="lz4 zlib zstd"
CMAKE_ARGS_EXTRA=( -DROCKSDB_BUILD_SHARED=ON -DWITH_GFLAGS=OFF -DWITH_TESTS=OFF -DWITH_ALL_TESTS=OFF
                   -DWITH_BENCHMARK_TOOLS=OFF -DWITH_TRACE_TOOLS=OFF -DWITH_CORE_TOOLS=ON -DWITH_TOOLS=OFF
                   -DWITH_SNAPPY=OFF -DWITH_LZ4=ON -DWITH_ZLIB=ON -DWITH_ZSTD=ON -DWITH_BZ2=OFF
                   -DWITH_LIBURING=OFF -DWITH_JEMALLOC=OFF -DWITH_NUMA=OFF -DWITH_JNI=OFF
                   -DPORTABLE=1 -DFAIL_ON_WARNINGS=OFF -DUSE_RTTI=1 -DROCKSDB_NO_FAST_LINKER=ON )
# The core tools (ldb, sst_dump) are built but have no install rule.
do_install() {
  DESTDIR="$STAGE" cmake --install "$B"
  install -Dm755 "$B/tools/ldb" "$STAGE$PREFIX/bin/ldb"
  install -Dm755 "$B/tools/sst_dump" "$STAGE$PREFIX/bin/sst_dump"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
