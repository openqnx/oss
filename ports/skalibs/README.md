# skalibs 2.14.5.1

> Set of general-purpose C programming libraries for skarnet.org software

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/skarnet/skalibs.git (tag `v2.14.5.1`) |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_skalibs`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `skalibs` (programs, shared libraries, run-time data) and `skalibs-dev` (headers, pkg-config/CMake files, static libraries; needs `skalibs`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`skalibs`](https://openqnx.github.io/oss/package/x86_64/skalibs.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/skalibs.html)) · [`skalibs-dev`](https://openqnx.github.io/oss/package/x86_64/skalibs-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/skalibs-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add skalibs` (dependencies come along); one version: `apk add skalibs=<version>` | `apk del skalibs` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install skalibs` (dependencies first) | `oss-get remove skalibs` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`skalibs-dev` the same way, by its own name (`apk add skalibs-dev`, `oss-get install skalibs-dev`, by hand with its own file and list `skalibs-dev.list`); remove it before `skalibs`, which it needs.

```sh
# on the QNX target, as root
apk add skalibs skalibs-dev
apk add qnx-test-skalibs && /usr/local/bin/qnx-port-tests/test_skalibs    # its on-target test, from tests-8.0-<arch>-m-z
apk del skalibs skalibs-dev qnx-test-skalibs
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/skalibs.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/skalibs.list
rm var/lib/manual/skalibs.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild skalibs` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2.14.5.1 --recurse-submodules --shallow-submodules \
    https://github.com/skarnet/skalibs.git src/skalibs
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/skalibs < ports/skalibs/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/skalibs < ports/skalibs/patches/0002-sig-catch-no-sa-restart.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — add sys/param.h in places to look for endianness

> Imported from qnx-ports/skalibs (branch qnx-release-v2.14.5.1, 8d27271..d9a27fa); original authorship kept.
>
> ---
>  configure | 2 +-
>  1 file changed, 1 insertion(+), 1 deletion(-)

Touches: `configure` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-sig-catch-no-sa-restart.patch`](patches/0002-sig-catch-no-sa-restart.patch)** — sig_catch: define SA_RESTART to 0 where the system lacks it

> QNX does not implement SA_RESTART (its <signal.h> has it commented out
> as "not supported yet"). Without it an interrupted system call returns
> EINTR, which skalibs' I/O primitives already retry on (the safe wrappers
> loop on EINTR), so 0 is the honest value.

Touches: `src/libstddjb/sig_catch.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/skalibs; B=build/$QNX_ARCH/skalibs; mkdir -p $B
```

```bash
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && \
  CC="$QNX_TRIPLE-gcc" CFLAGS="-O2 -D_QNX_SOURCE" LDFLAGS="-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state" \
  ./configure --target="$QNX_TRIPLE" --prefix="$PREFIX" --enable-shared --enable-static --enable-pkgconfig \
    --with-sysdep-devurandom=yes --with-sysdep-posixspawnearlyreturn=no --with-sysdep-procselfexe=none
}
```

```bash
do_build() { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- skalibs, skarnet's general-purpose C library (buffers, stralloc, iopause, selfpipe, TAI time, djb-style unix utilities) that s6, execline and the rest of the skarnet software are built on. 2.14.5.1 with qnx-ports' QNX change (patches/0001: configure finds the byte order in <sys/param.h>, where QNX keeps BYTE_ORDER -- there is no <endian.h>); patches/0002: SA_RESTART, which QNX does not implement, is 0 in sig_catch().
- skalibs' configure is hand-written and works in the source tree only; it is driven with the cross-GCC (in PATH from env.sh) and the target triple, so it finds the matching ar/ranlib/strip by prefix. The three sysdeps it cannot detect when cross-compiling: /dev/urandom works (QNX's random service); posix_spawn() reports exec failure (no early return); and there is no readlink()-able /proc/self/exe (QNX's /proc/self/exefile is a plain file), so procselfexe=none. Everything else is probed by compiling and linking against the SDP.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH skalibs`.

### 5. What it installs

103 files under `/usr/local`: 3 shared-library entries, 90 headers.

```
/usr/local/lib/libskarnet.so
/usr/local/lib/libskarnet.so.2.14
/usr/local/lib/libskarnet.so.2.14.5.1
```

The complete list is `stage/<arch>/.manifest/skalibs` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa skalibs
ls dists/8.0/*/ | grep '^skalibs-'
```

This writes `skalibs-<ver>-r0.tar.gz` (run time) and `skalibs-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add skalibs
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/skalibs-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/skalibs-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_skalibs`](../../tests/src/) links skalibs the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-skalibs`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-skalibs
/usr/local/bin/qnx-port-tests/test_skalibs
```

```
== skalibs ==
  ok   ...
PASS skalibs
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_skalibs` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_skalibs`](../../tests/src/) -- a program linked against skalibs the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - ulong_fmt(2^32) = 4294967296
   - ulong_xfmt(0xdeadbeef)
   - ulong_scan stops after 9 digits
   - uint32_pack_big byte order
   - uint32_unpack (little endian) of 01 02 03 04
   - ip4_scan
   - ip4_fmt round trip
   - stralloc_cats
   - stralloc contents and length
   - SHA-256(\"abc\") known answer
   - scratch directory
   - cdbmake: 3 records written
   - cdb_init (mapped)
   - cdb_find(os) = QNX
   - cdb_findnext: two values for key arch
   - cdb_find(missing) = 0
   - openwritenclose_unsafe
   - openslurpclose reads it back
   - selfpipe_init
   - selfpipe_trap(SIGUSR1)
   - iopause: selfpipe readable
   - selfpipe_read() = SIGUSR1
   - iopause: times out at its deadline (~200 ms)
   - child_spawn0
   - child exit status 5

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | skalibs 2.14.5.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/skalibs-2.14.5.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | skalibs 2.14.5.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/skalibs-2.14.5.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.14.5.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add skalibs=<version>`), sources in `ported/skalibs/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates skalibs`; port one with `scripts/port-update skalibs`.

<details><summary>recipe.sh</summary>

```bash
# skalibs, skarnet's general-purpose C library (buffers, stralloc, iopause,
# selfpipe, TAI time, djb-style unix utilities) that s6, execline and the
# rest of the skarnet software are built on. 2.14.5.1 with qnx-ports' QNX
# change (patches/0001: configure finds the byte order in <sys/param.h>,
# where QNX keeps BYTE_ORDER -- there is no <endian.h>); patches/0002:
# SA_RESTART, which QNX does not implement, is 0 in sig_catch().
VERSION=2.14.5.1; GIT=https://github.com/skarnet/skalibs.git; REF=v2.14.5.1
BUILDSYS=custom
# skalibs' configure is hand-written and works in the source tree only; it
# is driven with the cross-GCC (in PATH from env.sh) and the target triple,
# so it finds the matching ar/ranlib/strip by prefix. The three sysdeps it
# cannot detect when cross-compiling: /dev/urandom works (QNX's random
# service); posix_spawn() reports exec failure (no early return); and there
# is no readlink()-able /proc/self/exe (QNX's /proc/self/exefile is a plain
# file), so procselfexe=none. Everything else is probed by compiling and
# linking against the SDP.
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && \
  CC="$QNX_TRIPLE-gcc" CFLAGS="-O2 -D_QNX_SOURCE" LDFLAGS="-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state" \
  ./configure --target="$QNX_TRIPLE" --prefix="$PREFIX" --enable-shared --enable-static --enable-pkgconfig \
    --with-sysdep-devurandom=yes --with-sysdep-posixspawnearlyreturn=no --with-sysdep-procselfexe=none
}
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
