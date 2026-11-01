# dhcpcd 10.5.2

> DHCP, DHCPv6 and IPv4LL (ZeroConf) client

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/NetworkConfiguration/dhcpcd/releases/download/v10.5.2/dhcpcd-10.5.2.tar.xz |
| SHA-256 | `3e476657fdb6eeb38b277da3a48d0ac0113ecce5858ebdcddff2b629faed52b4` |
| Build system | custom (see recipe) |
| QNX patches | 5 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_dhcpcd`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `dhcpcd` (programs, shared libraries, run-time data) and `dhcpcd-dev` (headers, pkg-config/CMake files, static libraries; needs `dhcpcd`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`dhcpcd`](https://openqnx.github.io/oss/package/x86_64/dhcpcd.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/dhcpcd.html)) · [`dhcpcd-dev`](https://openqnx.github.io/oss/package/x86_64/dhcpcd-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/dhcpcd-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add dhcpcd` (dependencies come along); one version: `apk add dhcpcd=<version>` | `apk del dhcpcd` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install dhcpcd` (dependencies first) | `oss-get remove dhcpcd` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`dhcpcd-dev` the same way, by its own name (`apk add dhcpcd-dev`, `oss-get install dhcpcd-dev`, by hand with its own file and list `dhcpcd-dev.list`); remove it before `dhcpcd`, which it needs.

```sh
# on the QNX target, as root
apk add dhcpcd dhcpcd-dev
apk add qnx-test-dhcpcd && /usr/local/bin/qnx-port-tests/test_dhcpcd    # its on-target test, from tests-8.0-<arch>
apk del dhcpcd dhcpcd-dev qnx-test-dhcpcd
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/dhcpcd.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/dhcpcd.list
rm var/lib/manual/dhcpcd.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild dhcpcd` performs exactly the same steps (2–5) in one command.

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
URL=https://github.com/NetworkConfiguration/dhcpcd/releases/download/v10.5.2/dhcpcd-10.5.2.tar.xz
mkdir -p dist src/dhcpcd
curl -fL -o dist/$(basename $URL) $URL
echo "3e476657fdb6eeb38b277da3a48d0ac0113ecce5858ebdcddff2b629faed52b4  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/dhcpcd --strip-components=1
```

### 3. Apply the QNX patches

5 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/dhcpcd < ports/dhcpcd/patches/0001-qnx-is-bsd.patch
patch -p1 -d src/dhcpcd < ports/dhcpcd/patches/0002-getpeereid-libsocket.patch
patch -p1 -d src/dhcpcd < ports/dhcpcd/patches/0003-qnx-eloop-ppoll.patch
patch -p1 -d src/dhcpcd < ports/dhcpcd/patches/0004-arc4random-h-size-t.patch
patch -p1 -d src/dhcpcd < ports/dhcpcd/patches/0005-logfile-setvbuf.patch
```

**[`0001-qnx-is-bsd.patch`](patches/0001-qnx-is-bsd.patch)** — configure: define BSD on QNX, whose network stack is FreeBSD's.

> QNX 8.0's network stack, io-sock, is FreeBSD's: interfaces and addresses
> come from route sockets (struct rt_msghdr, RTM_NEWADDR, ...), link-level
> addresses are sockaddr_dl, and raw frames go through BPF. dhcpcd's if-bsd.c
> and bpf-bsd.c handle exactly that, but the rest of the code selects them
> with #ifdef BSD, which <sys/param.h> defines on the BSDs and QNX does not:
> the QNX case in configure already builds if-bsd.c, and without BSD the BSD
> parts of struct dhcpcd_ctx and struct priv are missing and if-bsd.c does
> not compile. Define BSD for QNX in configure.

Touches: `configure` · Upstream status: QNX-specific

**[`0002-getpeereid-libsocket.patch`](patches/0002-getpeereid-libsocket.patch)** — configure: link the getpeereid probe with -lsocket where needed.

> On QNX getpeereid() is in libsocket, not libc, like getifaddrs() and
> inet_ntoa(), whose probes already retry with -lsocket. The getpeereid probe
> linked without it, failed, and configure compiled compat/getpeereid.c, which
> stops with "OS has no getpeereid support!". Link the probe with the socket
> library found by the getifaddrs probe.

Touches: `configure` · Upstream status: QNX-specific

**[`0003-qnx-eloop-ppoll.patch`](patches/0003-qnx-eloop-ppoll.patch)** — eloop: use ppoll(2) on QNX, which has no kqueue.

> eloop.c uses kqueue(2) whenever BSD is defined. QNX defines BSD for its
> FreeBSD network stack (previous patch) but has no kqueue (no <sys/event.h>);
> it has ppoll(2), dhcpcd's portable backend. Take that on QNX.

Touches: `src/eloop.c` · Upstream status: QNX-specific

**[`0004-arc4random-h-size-t.patch`](patches/0004-arc4random-h-size-t.patch)** — compat/arc4random.h: include <stddef.h> for size_t.

> compat/arc4random.h declares arc4random_buf(void *, size_t) after including
> only <stdint.h>, which does not define size_t. config.h includes it before
> anything else when the C library has no arc4random() (QNX), and every file
> then failed with "unknown type name 'size_t'". Include <stddef.h>.

Touches: `compat/arc4random.h` · Upstream status: QNX-specific

**[`0005-logfile-setvbuf.patch`](patches/0005-logfile-setvbuf.patch)** — logerr: line-buffer the log file with setvbuf(), not setlinebuf().

> setlinebuf() is a BSD function QNX's <stdio.h> does not declare (the C
> library exports it, so the implicit declaration linked). setvbuf() with
> _IOLBF is the standard spelling of the same thing.

Touches: `src/logerr.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/dhcpcd; B=build/$QNX_ARCH/dhcpcd; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/" && cd "$B" && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
  ./configure --host="$QNX_TRIPLE" --os=qnx --prefix="$PREFIX" \
     --sysconfdir="$PREFIX/etc" --libexecdir="$PREFIX/libexec" \
     --dbdir=/var/db/dhcpcd --runstatedir=/var/run \
     --disable-privsep --without-openssl --with-hooks=; }
```

```bash
do_build() { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- dhcpcd's configure is hand-written (not autoconf) and builds in its source directory, so the tree is copied into the build directory first.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH dhcpcd`.

### 5. What it installs

12 files under `/usr/local`, 1 programs.

```
/usr/local/sbin/dhcpcd
```

The complete list is `stage/<arch>/.manifest/dhcpcd` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa dhcpcd
ls dists/8.0/*/ | grep '^dhcpcd-'
```

This writes `dhcpcd-<ver>-r0.tar.gz` (run time) and `dhcpcd-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add dhcpcd
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/dhcpcd-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/dhcpcd-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_dhcpcd`](../../tests/src/) links dhcpcd the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-dhcpcd`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-dhcpcd
/usr/local/bin/qnx-port-tests/test_dhcpcd
```

```
== dhcpcd ==
  ok   ...
PASS dhcpcd
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_dhcpcd` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_dhcpcd`](../../tests/src/) -- a program linked against dhcpcd the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | dhcpcd 10.5.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/dhcpcd-10.5.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | dhcpcd 10.5.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/dhcpcd-10.5.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **10.5.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add dhcpcd=<version>`), sources in `ported/dhcpcd/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates dhcpcd`; port one with `scripts/port-update dhcpcd`.

<details><summary>recipe.sh</summary>

```bash
VERSION=10.5.2
TARBALL=https://github.com/NetworkConfiguration/dhcpcd/releases/download/v10.5.2/dhcpcd-10.5.2.tar.xz
SHA256=3e476657fdb6eeb38b277da3a48d0ac0113ecce5858ebdcddff2b629faed52b4
BUILDSYS=custom
# dhcpcd's configure is hand-written (not autoconf) and builds in its source
# directory, so the tree is copied into the build directory first.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
  ./configure --host="$QNX_TRIPLE" --os=qnx --prefix="$PREFIX" \
     --sysconfdir="$PREFIX/etc" --libexecdir="$PREFIX/libexec" \
     --dbdir=/var/db/dhcpcd --runstatedir=/var/run \
     --disable-privsep --without-openssl --with-hooks=; }
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
