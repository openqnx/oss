# e2fsprogs 1.47.4

> Ext2/3/4 filesystem utilities

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://mirrors.edge.kernel.org/pub/linux/kernel/people/tytso/e2fsprogs/v1.47.4/e2fsprogs-1.47.4.tar.xz |
| SHA-256 | `fd5bf388cbdbe006a3d3b318d983b2948382440acc85a87f1e7d108653e8db0b` |
| Build system | GNU Autotools |
| QNX patches | 3 |
| Build dependencies | [`libuuid`](../libuuid/) |
| Run-time dependencies | [`libuuid`](../libuuid/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_e2fsprogs`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `e2fsprogs` (programs, shared libraries, run-time data) and `e2fsprogs-dev` (headers, pkg-config/CMake files, static libraries; needs `e2fsprogs`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`e2fsprogs`](https://openqnx.github.io/oss/package/x86_64/e2fsprogs.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/e2fsprogs.html)) · [`e2fsprogs-dev`](https://openqnx.github.io/oss/package/x86_64/e2fsprogs-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/e2fsprogs-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add e2fsprogs` (dependencies come along); one version: `apk add e2fsprogs=<version>` | `apk del e2fsprogs` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install e2fsprogs` (dependencies first) | `oss-get remove e2fsprogs` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`e2fsprogs-dev` the same way, by its own name (`apk add e2fsprogs-dev`, `oss-get install e2fsprogs-dev`, by hand with its own file and list `e2fsprogs-dev.list`); remove it before `e2fsprogs`, which it needs.

```sh
# on the QNX target, as root
apk add e2fsprogs e2fsprogs-dev
apk add qnx-test-e2fsprogs && /usr/local/bin/qnx-port-tests/test_e2fsprogs    # its on-target test, from tests-8.0-<arch>
apk del e2fsprogs e2fsprogs-dev qnx-test-e2fsprogs
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/e2fsprogs.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/e2fsprogs.list
rm var/lib/manual/e2fsprogs.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild e2fsprogs` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libuuid
```

### 2. Get the upstream source

```bash
URL=https://mirrors.edge.kernel.org/pub/linux/kernel/people/tytso/e2fsprogs/v1.47.4/e2fsprogs-1.47.4.tar.xz
mkdir -p dist src/e2fsprogs
curl -fL -o dist/$(basename $URL) $URL
echo "fd5bf388cbdbe006a3d3b318d983b2948382440acc85a87f1e7d108653e8db0b  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/e2fsprogs --strip-components=1
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/e2fsprogs < ports/e2fsprogs/patches/0001-makedev-three-arguments.patch
patch -p1 -d src/e2fsprogs < ports/e2fsprogs/patches/0002-bmap64-include-sys-time.patch
patch -p1 -d src/e2fsprogs < ports/e2fsprogs/patches/0003-no-sbrk.patch
```

**[`0001-makedev-three-arguments.patch`](patches/0001-makedev-three-arguments.patch)** — Use the two-argument makedev() these callers mean on QNX.

> QNX's makedev() takes three arguments, (node, major, minor), and its
> <sys/types.h> defines it as a macro of that shape, so every two-argument
> makedev(major, minor) call fails to compile. Where QNX is the target, define
> the two-argument form these files mean: a device number on the local node,
> encoded the way QNX's own macro encodes major and minor.

Touches: `lib/blkid/devname.c`, `lib/blkid/devno.c`, `lib/ext2fs/finddev.c`, `debugfs/debugfs.c` · Upstream status: QNX-specific

**[`0002-bmap64-include-sys-time.patch`](patches/0002-bmap64-include-sys-time.patch)** — bmap64.h: include <sys/time.h> for struct timeval.

> struct ext2_bmap_statistics has a struct timeval member, but the header
> relied on <sys/time.h> having been included by whoever included it. On QNX
> nothing the libext2fs sources include before it declares struct timeval
> ("field 'created' has incomplete type"). Include <sys/time.h> here.

Touches: `lib/ext2fs/bmap64.h` · Upstream status: Pending

**[`0003-no-sbrk.patch`](patches/0003-no-sbrk.patch)** — Leave out the sbrk()-based memory bookkeeping on QNX.

> e2fsck's and resize2fs's resource tracking (e2fsck -t, resize2fs -d 8)
> records the start of the heap with sbrk(0). QNX has no sbrk(): its heap is
> not a brk segment, and the programs fail to link. e2fsck's tracking is
> compiled out on QNX; resize2fs's memory figure already comes from
> mallinfo() there, so only its sbrk(0) bookkeeping is left out.

Touches: `e2fsck/e2fsck.h`, `resize/resource_track.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/e2fsprogs && cd build/$QNX_ARCH/e2fsprogs
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
ac_cv_func_mbstowcs=no \
LDCONFIG=: \
$OLDPWD/src/e2fsprogs/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-elf-shlibs \
    --disable-nls \
    --disable-fuse2fs \
    --disable-uuidd \
    --disable-libuuid \
    --enable-libblkid \
    --disable-e2initrd-helper \
    --disable-defrag \
    --without-systemd-unit-dir \
    --without-udev-rules-dir \
    --without-crond-dir
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- e2fsprogs: the ext2/ext3/ext4 file system utilities (mke2fs, e2fsck, tune2fs, debugfs, dumpe2fs, resize2fs, e2label, ...) and their libraries, libext2fs, libe2p, libcom_err and libss.
- libuuid comes from its own port (util-linux's); e2fsprogs would otherwise install a private copy with the same soname and headers. Its libblkid stays (no other port provides one): --enable-libblkid builds the private one always. Left to configure, it takes any libblkid.so it finds in staging -- an aarch64le build linked one that a passing build had left there and no package provides, and every program failed at load with "Could not load library libblkid.so.1".
- badblocks measures its progress line with mbstowcs() + wcswidth() when configure finds mbstowcs(); QNX has no wcswidth(). Its only locale is "C", where strlen() is the width, which is what badblocks falls back to. LDCONFIG=: -- "make install" would otherwise run the build machine's ldconfig.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH e2fsprogs`.

### 5. What it installs

112 files under `/usr/local`: 15 shared-library entries, 28 programs, 18 headers.

```
/usr/local/lib/libblkid.so
/usr/local/lib/libblkid.so.1
/usr/local/lib/libblkid.so.1.0
/usr/local/lib/libcom_err.so
/usr/local/lib/libcom_err.so.2
/usr/local/lib/libcom_err.so.2.1
/usr/local/lib/libe2p.so
/usr/local/lib/libe2p.so.2
/usr/local/lib/libe2p.so.2.3
/usr/local/lib/libext2fs.so
/usr/local/lib/libext2fs.so.2
/usr/local/lib/libext2fs.so.2.4
/usr/local/lib/libss.so
/usr/local/lib/libss.so.2
/usr/local/lib/libss.so.2.0
/usr/local/bin/chattr
...
```

The complete list is `stage/<arch>/.manifest/e2fsprogs` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa e2fsprogs
ls dists/8.0/*/ | grep '^e2fsprogs-'
```

This writes `e2fsprogs-<ver>-r0.tar.gz` (run time) and `e2fsprogs-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add e2fsprogs
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/e2fsprogs-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/e2fsprogs-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_e2fsprogs`](../../tests/src/) links e2fsprogs the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-e2fsprogs`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-e2fsprogs
/usr/local/bin/qnx-port-tests/test_e2fsprogs
```

```
== e2fsprogs ==
  ok   ...
PASS e2fsprogs
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_e2fsprogs` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_e2fsprogs`](../../tests/src/) -- a program linked against e2fsprogs the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - com_err: EXT2_ET_BAD_MAGIC = \"Bad magic number in super-block\"
   - com_err: EXT2_ET_FILE_NOT_FOUND = \"File not found by ext2_lookup\"
   - ss_create_invocation
   - ss_execute_line: greet hello world
   - ss_execute_line: alias with a quoted argument
   - unknown command: SS_ET_COMMAND_NOT_FOUND
   - scratch directory
   - 16 MiB image file
   - mke2fs -t ext4 -b 1024 -L qnxtest -U <uuid> on the image
   - dumpe2fs -h
   - dumpe2fs: volume name qnxtest
   - dumpe2fs: the UUID given
   - dumpe2fs: 16384 blocks of 1024 bytes
   - dumpe2fs: ext4 features (extent, has_journal)
   - ext2fs_open on the image
   - libext2fs: magic 0xEF53, 16384 blocks, 1024-byte blocks, label qnxtest
   - libext2fs: lost+found is inode 11
   - ext2fs_open on something that is not a file system fails
   - blkid: TYPE=ext4, LABEL=qnxtest, the UUID
   - debugfs -w: write hello.txt, mkdir docs
   - debugfs cat: the file reads back identical
   - debugfs ls: hello.txt (10500 bytes) and docs
   - libext2fs: ext2fs_file_read of hello.txt, 10500 bytes as written
   - ext2fs_open after debugfs
   - e2fsck -fn: clean (exit 0)
   - ... and 7 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | e2fsprogs 1.47.4-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/e2fsprogs-1.47.4-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | e2fsprogs 1.47.4-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/e2fsprogs-1.47.4-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.47.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add e2fsprogs=<version>`), sources in `ported/e2fsprogs/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates e2fsprogs`; port one with `scripts/port-update e2fsprogs`.

<details><summary>recipe.sh</summary>

```bash
# e2fsprogs: the ext2/ext3/ext4 file system utilities (mke2fs, e2fsck,
# tune2fs, debugfs, dumpe2fs, resize2fs, e2label, ...) and their libraries,
# libext2fs, libe2p, libcom_err and libss.
VERSION=1.47.4
TARBALL=https://mirrors.edge.kernel.org/pub/linux/kernel/people/tytso/e2fsprogs/v1.47.4/e2fsprogs-1.47.4.tar.xz
SHA256=fd5bf388cbdbe006a3d3b318d983b2948382440acc85a87f1e7d108653e8db0b
BUILDSYS=autotools
# libuuid comes from its own port (util-linux's); e2fsprogs would otherwise
# install a private copy with the same soname and headers. Its libblkid stays
# (no other port provides one): --enable-libblkid builds the private one
# always. Left to configure, it takes any libblkid.so it finds in staging --
# an aarch64le build linked one that a passing build had left there and no
# package provides, and every program failed at load with "Could not load
# library libblkid.so.1".
DEPS="libuuid"
CONF_ARGS_EXTRA=( --enable-elf-shlibs --disable-nls --disable-fuse2fs --disable-uuidd --disable-libuuid --enable-libblkid
                  --disable-e2initrd-helper --disable-defrag --without-systemd-unit-dir
                  --without-udev-rules-dir --without-crond-dir )
# badblocks measures its progress line with mbstowcs() + wcswidth() when
# configure finds mbstowcs(); QNX has no wcswidth(). Its only locale is "C",
# where strlen() is the width, which is what badblocks falls back to.
# LDCONFIG=: -- "make install" would otherwise run the build machine's ldconfig.
CONF_ENV=( ac_cv_func_mbstowcs=no LDCONFIG=: )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
