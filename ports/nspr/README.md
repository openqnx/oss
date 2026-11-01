# nspr 4.39

> Netscape Portable Runtime

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://ftp.mozilla.org/pub/nspr/releases/v4.39/src/nspr-4.39.tar.gz |
| SHA-256 | `bbd02ee87a55676063a63e5bc819e0227de2666b47307b2a0134414cdf42368e` |
| Build system | GNU Autotools |
| QNX patches | 3 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_nspr`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `nspr` (programs, shared libraries, run-time data) and `nspr-dev` (headers, pkg-config/CMake files, static libraries; needs `nspr`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`nspr`](https://openqnx.github.io/oss/package/x86_64/nspr.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/nspr.html)) · [`nspr-dev`](https://openqnx.github.io/oss/package/x86_64/nspr-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/nspr-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add nspr` (dependencies come along); one version: `apk add nspr=<version>` | `apk del nspr` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install nspr` (dependencies first) | `oss-get remove nspr` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`nspr-dev` the same way, by its own name (`apk add nspr-dev`, `oss-get install nspr-dev`, by hand with its own file and list `nspr-dev.list`); remove it before `nspr`, which it needs.

```sh
# on the QNX target, as root
apk add nspr nspr-dev
apk add qnx-test-nspr && /usr/local/bin/qnx-port-tests/test_nspr    # its on-target test, from tests-8.0-<arch>-m-z
apk del nspr nspr-dev qnx-test-nspr
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/nspr.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/nspr.list
rm var/lib/manual/nspr.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild nspr` performs exactly the same steps (2–5) in one command.

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
URL=https://ftp.mozilla.org/pub/nspr/releases/v4.39/src/nspr-4.39.tar.gz
mkdir -p dist src/nspr
curl -fL -o dist/$(basename $URL) $URL
echo "bbd02ee87a55676063a63e5bc819e0227de2666b47307b2a0134414cdf42368e  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/nspr --strip-components=1
```

The project lives in the `nspr/` subdirectory of the repository.

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/nspr < ports/nspr/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/nspr < ports/nspr/patches/0002-nto-socklen-t.patch
patch -p1 -d src/nspr < ports/nspr/patches/0003-nto-ptio-poll-h.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — qnx support; add x86_64 support for qnx

> Imported from qnx-ports/nspr (396c53cad4db "release tarball for 4.38.2"
> ...23e88e260d2d, branch qnx-4.38.x); original authorship kept. Applies to
> 4.39 unchanged (offsets only).
>
> NSPR's NTO (QNX) machine layer dates from QNX 6 on x86: its _nto.cfg knows
> only i386 and _nto.h reports architecture "x86", disables poll() and large
> files. QNX 8.0 has poll(), 64-bit off_t and the 5-argument getprotobyname_r /
> int-returning gethostbyname_r; this adds the aarch64 and x86_64 type layouts,
> enables poll and large files, includes <spawn.h> for posix_spawn, and selects
> the reentrant netdb variants QNX provides.
>
> ---

Touches: `nspr/pr/include/md/_nto.cfg`, `nspr/pr/include/md/_nto.h`, `nspr/pr/src/md/unix/uxproces.c`, `nspr/pr/src/misc/prnetdb.c` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-nto-socklen-t.patch`](patches/0002-nto-socklen-t.patch)** — Define HAVE_SOCKLEN_T for QNX (NTO).

> QNX has socklen_t, but NSPR's configure defines HAVE_SOCKLEN_T only for the
> systems it lists, and unix.c then falls through to "#error Cannot determine
> architecture" (its NTO branch below is mangled). Define it for *-nto*.

Touches: `nspr/configure`, `nspr/configure.in` · Upstream status: QNX-specific

**[`0003-nto-ptio-poll-h.patch`](patches/0003-nto-ptio-poll-h.patch)** — Include <poll.h> in ptio.c on QNX (NTO).

> ptio.c includes <poll.h> only under _PR_POLL_AVAILABLE, which is defined in
> the machine header that is read later (via primpl.h). On Linux and the BSDs
> other system headers pull in poll.h; on QNX nothing does, and struct pollfd
> and POLLIN are undeclared. Include it for NTO unconditionally.

Touches: `nspr/pr/src/pthreads/ptio.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/nspr && cd build/$QNX_ARCH/nspr
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
HOST_CC=gcc \
HOST_CFLAGS=-O2 \
HOST_LDFLAGS= \
$OLDPWD/src/nspr/nspr/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --target="$QNX_TRIPLE" \
    --enable-64bit \
    --with-pthreads \
    --enable-optimize=-O2 \
    --disable-debug
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- NSPR, the Netscape Portable Runtime (threads, I/O, sockets, time) under NSS. NSS 3.131.1 needs NSPR 4.39; the QNX changes are qnx-ports' (made for 4.38.2, they apply to 4.39 unchanged).
- The release archive holds the autoconf project in nspr/.
- now and nsinstall run on the build machine during the build: without HOST_CC configure compiles them with the cross compiler.
- --target: NSPR's configure picks the machine layer (NTO, _nto.cfg) from $target. --enable-64bit: otherwise it assumes a 32-bit target and the generated prcpucfg.h contradicts the compiler.
- The archive's config.sub (2020) rejects the *-nto-qnx8.0.0 triples; use automake's current one (configure itself is shipped generated and kept).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH nspr`.

### 5. What it installs

78 files under `/usr/local`: 3 shared-library entries, 3 programs, 67 headers.

```
/usr/local/lib/libnspr4.so
/usr/local/lib/libplc4.so
/usr/local/lib/libplds4.so
/usr/local/bin/compile-et.pl
/usr/local/bin/nspr-config
/usr/local/bin/prerr.properties
```

The complete list is `stage/<arch>/.manifest/nspr` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa nspr
ls dists/8.0/*/ | grep '^nspr-'
```

This writes `nspr-<ver>-r0.tar.gz` (run time) and `nspr-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add nspr
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/nspr-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/nspr-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_nspr`](../../tests/src/) links nspr the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-nspr`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-nspr
/usr/local/bin/qnx-port-tests/test_nspr
```

```
== nspr ==
  ok   ...
PASS nspr
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_nspr` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_nspr`](../../tests/src/) -- a program linked against nspr the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - PR_Init
   - PR_snprintf formats
   - PR_smprintf allocates
   - PR_ExplodeTime(1700000000) = Tue 2023-11-14 22:13:20 UTC
   - PR_ImplodeTime round trip
   - PR_ParseTimeString (RFC 1123 date)
   - PR_FormatTimeUSEnglish
   - PR_Now is after 2023
   - PL_Base64Encode
   - PL_Base64Decode
   - PL_strcasecmp / PL_strlen
   - PLHashTable lookup
   - PLHashTable remove
   - 4 threads x 10000 locked increments = 40000
   - condvar producer/consumer: 50 items, sum 1275
   - TCP listen on 127.0.0.1 (ephemeral port)
   - PR_Connect
   - TCP echo round trip
   - PR_Poll times out with no data
   - EOF after the server closes
   - PR_StringToNetAddr / PR_NetAddrToString
   - PR_Cleanup

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | nspr 4.39-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/nspr-4.39-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | nspr 4.39-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/nspr-4.39-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.39**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add nspr=<version>`), sources in `ported/nspr/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates nspr`; port one with `scripts/port-update nspr`.

<details><summary>recipe.sh</summary>

```bash
# NSPR, the Netscape Portable Runtime (threads, I/O, sockets, time) under NSS.
# NSS 3.131.1 needs NSPR 4.39; the QNX changes are qnx-ports' (made for
# 4.38.2, they apply to 4.39 unchanged).
VERSION=4.39
TARBALL=https://ftp.mozilla.org/pub/nspr/releases/v4.39/src/nspr-4.39.tar.gz
SHA256=bbd02ee87a55676063a63e5bc819e0227de2666b47307b2a0134414cdf42368e
BUILDSYS=autotools
# The release archive holds the autoconf project in nspr/.
SUBDIR=nspr
# now and nsinstall run on the build machine during the build: without
# HOST_CC configure compiles them with the cross compiler.
CONF_ENV=( HOST_CC=gcc HOST_CFLAGS=-O2 HOST_LDFLAGS= )
# --target: NSPR's configure picks the machine layer (NTO, _nto.cfg) from
# $target. --enable-64bit: otherwise it assumes a 32-bit target and the
# generated prcpucfg.h contradicts the compiler.
CONF_ARGS_EXTRA=( --target="$QNX_TRIPLE" --enable-64bit --with-pthreads
                  --enable-optimize=-O2 --disable-debug )
# The archive's config.sub (2020) rejects the *-nto-qnx8.0.0 triples; use
# automake's current one (configure itself is shipped generated and kept).
do_patch() {
  local aux; aux=$(ls -d /usr/share/automake-*/ 2>/dev/null | head -1)
  [ -n "$aux" ] || { echo "no automake data directory found"; return 1; }
  cp -f "$aux/config.sub" "$aux/config.guess" "$SRCROOT/nspr/build/autoconf/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
