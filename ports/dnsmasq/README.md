# dnsmasq 2.93test4

> Lightweight DNS forwarder and DHCP server

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://thekelleys.org.uk/git/dnsmasq.git (tag `v2.93test4`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_dnsmasq`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add dnsmasq
apk add dnsmasq-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-dnsmasq && /usr/local/bin/qnx-port-tests/test_dnsmasq    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install dnsmasq`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild dnsmasq` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2.93test4 --recurse-submodules --shallow-submodules \
    https://thekelleys.org.uk/git/dnsmasq.git src/dnsmasq
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/dnsmasq < ports/dnsmasq/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Port dnsmasq for QNX 8.0

> Imported from qnx-ports/dnsmasq (e4d0bb45799a...qnx-v2.92); original authorship kept.
>
>
> ---
>  src/bpf.c     | 2 +-
>  src/dhcp.c    | 2 +-
>  src/dnsmasq.h | 4 ++--
>  3 files changed, 4 insertions(+), 4 deletions(-)

Touches: `src/bpf.c`, `src/dhcp.c`, `src/dnsmasq.h`, `src/bpf.c`, `src/config.h` · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/dnsmasq; B=build/$QNX_ARCH/dnsmasq; mkdir -p $B
```

```bash
do_build() { cp -a "$S/." "$B/" && _dnsmasq_make; }
```

```bash
do_install() { _dnsmasq_make DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The same make variables for "install": its target depends on "all", and without CC there it rebuilt dnsmasq with the build host's cc -- both architectures shipped an x86-64 Linux executable (scripts/check-artifacts).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH dnsmasq`.

### 5. What it installs

2 files under `/usr/local`, 1 programs.

```
/usr/local/sbin/dnsmasq
```

The complete list is `stage/<arch>/.manifest/dnsmasq` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa dnsmasq
ls dists/8.0/*/ | grep '^dnsmasq-'
```

This writes `dnsmasq-<ver>-r0.tar.gz` (run time) and `dnsmasq-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add dnsmasq
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/dnsmasq-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/dnsmasq-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_dnsmasq`](../../tests/src/) links dnsmasq the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-dnsmasq`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-dnsmasq
/usr/local/bin/qnx-port-tests/test_dnsmasq
```

```
== dnsmasq ==
  ok   ...
PASS dnsmasq
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_dnsmasq` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_dnsmasq`](../../tests/src/) -- a program linked against dnsmasq the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - sbin/dnsmasq installed
   - started
   - UDP: box.lan A 192.168.7.7 (--host-record)
   - deep.sub.qnx.test A 10.1.2.3 (--address for the domain)
   - PTR 192.168.7.7 -> box.lan
   - alias.lan: CNAME box.lan, then its A record
   - info.lan TXT \"hello from dnsmasq\"
   - nothing.lan: NXDOMAIN (a --local domain, no upstream asked)
   - TCP: box.lan A 192.168.7.7

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | dnsmasq 2.93_pre4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/dnsmasq-2.93_pre4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | dnsmasq 2.93_pre4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/dnsmasq-2.93_pre4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.93test4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add dnsmasq=<version>`), sources in `ported/dnsmasq/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates dnsmasq`; port one with `scripts/port-update dnsmasq`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.93test4; GIT=https://thekelleys.org.uk/git/dnsmasq.git; REF=v2.93test4
BUILDSYS=custom
# The same make variables for "install": its target depends on "all", and
# without CC there it rebuilt dnsmasq with the build host's cc -- both
# architectures shipped an x86-64 Linux executable (scripts/check-artifacts).
_dnsmasq_make() { make -C "$B" CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -D_QNX_SOURCE" LDFLAGS="-lsocket -lm" PREFIX="$PREFIX" "$@"; }
do_build() { cp -a "$S/." "$B/" && _dnsmasq_make; }
do_install() { _dnsmasq_make DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
