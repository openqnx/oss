# netmap 14.0-git-20261027

> netmap apps (pkt-gen, bridge, lb, nmreplay, tlem) for io-sock's netmap

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/luigirizzo/netmap.git |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_netmap`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `netmap` (programs, shared libraries, run-time data) and `netmap-dev` (headers, pkg-config/CMake files, static libraries; needs `netmap`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`netmap`](https://openqnx.github.io/oss/package/x86_64/netmap.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/netmap.html)) · [`netmap-dev`](https://openqnx.github.io/oss/package/x86_64/netmap-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/netmap-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add netmap` (dependencies come along); one version: `apk add netmap=<version>` | `apk del netmap` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install netmap` (dependencies first) | `oss-get remove netmap` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`netmap-dev` the same way, by its own name (`apk add netmap-dev`, `oss-get install netmap-dev`, by hand with its own file and list `netmap-dev.list`); remove it before `netmap`, which it needs.

```sh
# on the QNX target, as root
apk add netmap netmap-dev
apk add qnx-test-netmap && /usr/local/bin/qnx-port-tests/test_netmap    # its on-target test, from tests-8.0-<arch>-m-z
apk del netmap netmap-dev qnx-test-netmap
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/netmap.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/netmap.list
rm var/lib/manual/netmap.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild netmap` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/luigirizzo/netmap.git src/netmap
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/netmap < ports/netmap/patches/0001-qnx-platform.patch
patch -p1 -d src/netmap < ports/netmap/patches/0002-lb-strchr.patch
```

**[`0001-qnx-platform.patch`](patches/0001-qnx-platform.patch)** — QNX platform support for the netmap applications.

> QNX 8.0 has netmap in io-sock, but the applications only know Linux,
> FreeBSD, macOS and Windows. Adds apps/include/qnx_compat.h, which the
> recipe force-includes on QNX: thread affinity through the thread's run mask
> (ThreadCtl(_NTO_TCTL_RUNMASK); QNX has no pthread_setaffinity_np()/cpuset_t,
> and the applications only pin the calling thread), bzero()/bcopy() (not
> declared by QNX 8's <strings.h>), CLOCK_REALTIME_PRECISE, MAP_HUGETLB (0:
> QNX has no huge-page mmap flag, as on FreeBSD) and <net/if_dl.h> for the
> link addresses getifaddrs() returns. In pkt-gen: the CPU count from
> sysconf(_SC_NPROCESSORS_ONLN) (QNX has no CTL_HW/HW_NCPU sysctl), and the
> FreeBSD-style tap clone device.

Touches: `apps/include/qnx_compat.h`, `apps/pkt-gen/pkt-gen.c` · Upstream status: QNX-specific

**[`0002-lb-strchr.patch`](patches/0002-lb-strchr.patch)** — lb: strchr() instead of index().

> index() is a legacy BSD function QNX's C library does not provide (the link
> fails with an undefined reference); strchr() is the standard equivalent.

Touches: `apps/lb/lb.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/netmap; B=build/$QNX_ARCH/netmap; mkdir -p $B
```

```bash
do_build() {
  local a A="$S/apps"
  # -DNO_PCAP: pkt-gen's pcap replay (-f) would make libpcap a run-time
  # dependency for one option; nmreplay replays pcap files without it.
  # The libraries: libnetmap (SDP), libsocket (getifaddrs, ether_aton),
  # libm (tlem, pkt-gen statistics).
  for a in pkt-gen bridge nmreplay tlem; do
    nm_cc -DNO_PCAP -I"$A/include" "$A/$a/$a.c" -o "$B/$a" -lnetmap -lsocket -lm || return 1
  done
  nm_cc -I"$A/include" "$A/lb/lb.c" "$A/lb/pkt_hash.c" -o "$B/lb" -lnetmap -lsocket -lm
}
```

```bash
do_install() {
  local a s
  install -d "$STAGE$PREFIX/bin" "$STAGE$PREFIX/share/man/man8"
  for a in $NM_APPS; do
    install -m 755 "$B/$a" "$STAGE$PREFIX/bin/$a"
    s=8; install -m 644 "$S/apps/$a/$a.$s" "$STAGE$PREFIX/share/man/man8/$a.$s" || return 1
  done
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The netmap applications (pkt-gen, bridge, lb, nmreplay, tlem) from the upstream netmap tree. QNX 8.0's io-sock has netmap built in (/dev/netmap, VALE switches, netmap pipes) and the SDP ships the user-space side -- <net/netmap.h>, <net/netmap_user.h>, <libnetmap.h>, libnetmap.so.1 and vale-ctl -- but none of these applications. So only apps/ is built here, against the SDP's headers and libnetmap: the SDP's copies carry QNX's changes (io-sock instance prefix from $SOCK, rings mapped through a shared memory handle instead of mmap() of the device), which upstream's libnetmap does not have. vale-ctl is not built: the SDP has it. At run time the SDP's libnetmap.so.1 must be on the target (a stock mkqnximage image lacks it). With io-sock's default tunables a VALE port's memory allocator holds two netmap endpoints (dev.netmap.priv_if_num=2, priv_ring_num=4, priv_buf_num=4098): pkt-gen and nmreplay between two ports or over a pipe work; bridge and lb need more endpoints (raise those sysctls), and bridge between two VALE switches was seen not to be woken by poll() on QNX -- see the test notes.
- apps/include/qnx_compat.h (added by the patch) is force-included: the thread-affinity, bzero/bcopy and clock/mmap-flag differences, in one place.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH netmap`.

### 5. What it installs

10 files under `/usr/local`, 5 programs.

```
/usr/local/bin/bridge
/usr/local/bin/lb
/usr/local/bin/nmreplay
/usr/local/bin/pkt-gen
/usr/local/bin/tlem
```

The complete list is `stage/<arch>/.manifest/netmap` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa netmap
ls dists/8.0/*/ | grep '^netmap-'
```

This writes `netmap-<ver>-r0.tar.gz` (run time) and `netmap-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add netmap
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/netmap-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/netmap-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_netmap`](../../tests/src/) links netmap the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-netmap`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-netmap
/usr/local/bin/qnx-port-tests/test_netmap
```

```
== netmap ==
  ok   ...
PASS netmap
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_netmap` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_netmap`](../../tests/src/) -- a program linked against netmap the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | netmap 14.0_git20261027-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/netmap-14.0_git20261027-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | netmap 14.0_git20261027-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/netmap-14.0_git20261027-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **14.0-git-20261027**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add netmap=<version>`), sources in `ported/netmap/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates netmap`; port one with `scripts/port-update netmap`.

<details><summary>recipe.sh</summary>

```bash
# The netmap applications (pkt-gen, bridge, lb, nmreplay, tlem) from the
# upstream netmap tree. QNX 8.0's io-sock has netmap built in (/dev/netmap,
# VALE switches, netmap pipes) and the SDP ships the user-space side --
# <net/netmap.h>, <net/netmap_user.h>, <libnetmap.h>, libnetmap.so.1 and
# vale-ctl -- but none of these applications. So only apps/ is built here,
# against the SDP's headers and libnetmap: the SDP's copies carry QNX's
# changes (io-sock instance prefix from $SOCK, rings mapped through a shared
# memory handle instead of mmap() of the device), which upstream's
# libnetmap does not have. vale-ctl is not built: the SDP has it.
# At run time the SDP's libnetmap.so.1 must be on the target (a stock
# mkqnximage image lacks it). With io-sock's default tunables a VALE port's
# memory allocator holds two netmap endpoints (dev.netmap.priv_if_num=2,
# priv_ring_num=4, priv_buf_num=4098): pkt-gen and nmreplay between two
# ports or over a pipe work; bridge and lb need more endpoints (raise those
# sysctls), and bridge between two VALE switches was seen not to be woken
# by poll() on QNX -- see the test notes.
VERSION=14.0-git-20261027; GIT=https://github.com/luigirizzo/netmap.git; COMMIT=37ec6d91d4fd2d0bd3de375fbd2119c56812de57
BUILDSYS=custom
NM_APPS="pkt-gen bridge lb nmreplay tlem"
# apps/include/qnx_compat.h (added by the patch) is force-included: the
# thread-affinity, bzero/bcopy and clock/mmap-flag differences, in one place.
nm_cc() { "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall \
            -include "$S/apps/include/qnx_compat.h" "$@"; }
do_build() {
  local a A="$S/apps"
  # -DNO_PCAP: pkt-gen's pcap replay (-f) would make libpcap a run-time
  # dependency for one option; nmreplay replays pcap files without it.
  # The libraries: libnetmap (SDP), libsocket (getifaddrs, ether_aton),
  # libm (tlem, pkt-gen statistics).
  for a in pkt-gen bridge nmreplay tlem; do
    nm_cc -DNO_PCAP -I"$A/include" "$A/$a/$a.c" -o "$B/$a" -lnetmap -lsocket -lm || return 1
  done
  nm_cc -I"$A/include" "$A/lb/lb.c" "$A/lb/pkt_hash.c" -o "$B/lb" -lnetmap -lsocket -lm
}
do_install() {
  local a s
  install -d "$STAGE$PREFIX/bin" "$STAGE$PREFIX/share/man/man8"
  for a in $NM_APPS; do
    install -m 755 "$B/$a" "$STAGE$PREFIX/bin/$a"
    s=8; install -m 644 "$S/apps/$a/$a.$s" "$STAGE$PREFIX/share/man/man8/$a.$s" || return 1
  done
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
