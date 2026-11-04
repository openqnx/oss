# tdb 1.4.15

> Samba's trivial database: a small multi-process key/value store

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://download.samba.org/pub/tdb/tdb-1.4.15.tar.gz |
| SHA-256 | `fba09d8df1f1b9072aeae8e78b2bd43c5afef20b2f6deefa633aa14a377a8dd2` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_tdb`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `tdb` (programs, shared libraries, run-time data) and `tdb-dev` (headers, pkg-config/CMake files, static libraries; needs `tdb`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`tdb`](https://openqnx.github.io/oss/package/x86_64/tdb.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/tdb.html)) · [`tdb-dev`](https://openqnx.github.io/oss/package/x86_64/tdb-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/tdb-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add tdb` (dependencies come along); one version: `apk add tdb=<version>` | `apk del tdb` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install tdb` (dependencies first) | `oss-get remove tdb` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`tdb-dev` the same way, by its own name (`apk add tdb-dev`, `oss-get install tdb-dev`, by hand with its own file and list `tdb-dev.list`); remove it before `tdb`, which it needs.

```sh
# on the QNX target, as root
apk add tdb tdb-dev
apk add qnx-test-tdb && /usr/local/bin/qnx-port-tests/test_tdb    # its on-target test, from tests-8.0-<arch>-m-z
apk del tdb tdb-dev qnx-test-tdb
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/tdb.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/tdb.list
rm var/lib/manual/tdb.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild tdb` performs exactly the same steps (2–5) in one command.

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
URL=https://download.samba.org/pub/tdb/tdb-1.4.15.tar.gz
mkdir -p dist src/tdb
curl -fL -o dist/$(basename $URL) $URL
echo "fba09d8df1f1b9072aeae8e78b2bd43c5afef20b2f6deefa633aa14a377a8dd2  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/tdb --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/tdb < ports/tdb/patches/0001-qnx-no-mmap.patch
```

**[`0001-qnx-no-mmap.patch`](patches/0001-qnx-no-mmap.patch)** — Never mmap the database on QNX; use pread/pwrite.

> On QNX 8.0 a MAP_SHARED mapping of a file is not coherent with write()s
> by other processes (lib/replace's incoherent_mmap probe, run on the
> target, on both /tmp and a qnx6 /data); tdb's answer to that,
> HAVE_INCOHERENT_MMAP, is "everyone must always mmap" -- and QNX refuses
> to map a file beyond its end (ENXIO), which Samba hit at once
> ("tdb_mmap failed for size 16384 (No such device or address)": smbpasswd
> could not store the machine SID in secrets.tdb). The other coherent
> choice is that nobody maps: on QNX, tdb does all database I/O with
> pread/pwrite (as with TDB_NOMMAP), and the cross-answers say
> HAVE_INCOHERENT_MMAP: NO. The robust-mutex area (TDB_MUTEX_LOCKING) is
> mapped on its own and is unaffected.

Touches: `common/io.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/tdb; B=build/$QNX_ARCH/tdb; mkdir -p $B
```

```bash
do_build() {
  rm -rf "$B" && cp -a "$S" "$B" && cd "$B" &&
  cp "$WS/ports/tdb/cross-answers-$QNX_ARCH.txt" "$B/cross-answers.txt" &&
  _tdb_waf_env ./configure --prefix="$PREFIX" --disable-python --disable-rpath --disable-rpath-install --disable-tdb-mutex-locking \
    --cross-compile --cross-answers="$B/cross-answers.txt" --hostcc=gcc &&
  _tdb_waf_env make -j"$(nproc)"
}
```

```bash
do_install() { cd "$B" && _tdb_waf_env make install DESTDIR="$STAGE"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- tdb: Samba's "trivial database" -- a small, fast key/value store with transactions and multi-process locking, used by Samba and others -- with tdbtool, tdbdump, tdbrestore and tdbbackup. Standalone build of the Samba project's release (the copy samba bundles is built with samba).
- waf (Samba's buildtools) in cross mode: every configure question that needs a test program run is answered from ports/tdb/cross-answers-<arch>.txt, recorded once on QNX targets (see the comments there). waf builds in the source tree (bin/), so it runs in a copy. No Python module (the target Python bindings would need the cross Python's headers and ABI). --disable-tdb-mutex-locking: tdb's robust-mutex mode keeps process-shared mutexes in a mapped area of the database file; on QNX Samba's first opens of a database failed there ("tdb_mutex_init failed ...: File exists"), so tdb locks with fcntl() as it does wherever mutexes are off.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH tdb`.

### 5. What it installs

9 files under `/usr/local`: 3 shared-library entries, 4 programs, 1 headers.

```
/usr/local/lib/libtdb.so
/usr/local/lib/libtdb.so.1
/usr/local/lib/libtdb.so.1.4.15
/usr/local/bin/tdbbackup
/usr/local/bin/tdbdump
/usr/local/bin/tdbrestore
/usr/local/bin/tdbtool
```

The complete list is `stage/<arch>/.manifest/tdb` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa tdb
ls dists/8.0/*/ | grep '^tdb-'
```

This writes `tdb-<ver>-r0.tar.gz` (run time) and `tdb-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add tdb
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/tdb-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/tdb-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_tdb`](../../tests/src/) links tdb the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-tdb`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-tdb
/usr/local/bin/qnx-port-tests/test_tdb
```

```
== tdb ==
  ok   ...
PASS tdb
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_tdb` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_tdb`](../../tests/src/) -- a program linked against tdb the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - tdb_open creates the database
   - store/fetch
   - TDB_INSERT on an existing key: TDB_ERR_EXISTS
   - tdb_append: a + bc = abc
   - traverse: 102 records
   - delete one, the others intact
   - a cancelled transaction leaves nothing
   - transaction start/store/commit
   - a record locked by another process: tdb_chainlock_nonblock fails
   - after the child unlocks, the lock is ours
   - the child's write is seen by the parent
   - tdb_check: consistent
   - reopened: committed data persisted
   - tdbdump prints the records
   - tdbtool <db> show tx: 9 bytes, hex of \"committed\"
   - tdbbackup: the copy has all 102 records

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> tdb`.

## Versions

This recipe builds **1.4.15**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add tdb=<version>`), sources in `ported/tdb/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates tdb`; port one with `scripts/port-update tdb`.

<details><summary>recipe.sh</summary>

```bash
# tdb: Samba's "trivial database" -- a small, fast key/value store with
# transactions and multi-process locking, used by Samba and others -- with
# tdbtool, tdbdump, tdbrestore and tdbbackup. Standalone build of the
# Samba project's release (the copy samba bundles is built with samba).
VERSION=1.4.15
TARBALL=https://download.samba.org/pub/tdb/tdb-1.4.15.tar.gz
SHA256=fba09d8df1f1b9072aeae8e78b2bd43c5afef20b2f6deefa633aa14a377a8dd2
BUILDSYS=custom
# waf (Samba's buildtools) in cross mode: every configure question that
# needs a test program run is answered from ports/tdb/cross-answers-<arch>.txt,
# recorded once on QNX targets (see the comments there). waf builds in the
# source tree (bin/), so it runs in a copy. No Python module (the target
# Python bindings would need the cross Python's headers and ABI).
# --disable-tdb-mutex-locking: tdb's robust-mutex mode keeps process-shared
# mutexes in a mapped area of the database file; on QNX Samba's first
# opens of a database failed there ("tdb_mutex_init failed ...: File
# exists"), so tdb locks with fcntl() as it does wherever mutexes are off.
_tdb_waf_env() { env CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CFLAGS="-O2 -I$STAGE_SYSROOT$PREFIX/include" \
  LDFLAGS="-L$STAGE_SYSROOT$PREFIX/lib -lsocket" PYTHONHASHSEED=1 "$@"; }
do_build() {
  rm -rf "$B" && cp -a "$S" "$B" && cd "$B" &&
  cp "$WS/ports/tdb/cross-answers-$QNX_ARCH.txt" "$B/cross-answers.txt" &&
  _tdb_waf_env ./configure --prefix="$PREFIX" --disable-python --disable-rpath --disable-rpath-install --disable-tdb-mutex-locking \
    --cross-compile --cross-answers="$B/cross-answers.txt" --hostcc=gcc &&
  _tdb_waf_env make -j"$(nproc)"
}
do_install() { cd "$B" && _tdb_waf_env make install DESTDIR="$STAGE"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
