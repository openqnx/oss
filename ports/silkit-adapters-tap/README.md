# silkit-adapters-tap 1.2.0

> SIL Kit adapter connecting TAP network devices to SIL Kit Ethernet networks

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/vectorgrp/sil-kit-adapters-tap.git (tag `v1.2.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`silkit`](../silkit/) |
| Run-time dependencies | [`silkit`](../silkit/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_silkit_adapters_tap`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `silkit-adapters-tap`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`silkit-adapters-tap`](https://openqnx.github.io/oss/package/x86_64/silkit-adapters-tap.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/silkit-adapters-tap.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add silkit-adapters-tap` (dependencies come along); one version: `apk add silkit-adapters-tap=<version>` | `apk del silkit-adapters-tap` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install silkit-adapters-tap` (dependencies first) | `oss-get remove silkit-adapters-tap` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-silkit-adapters-tap && /usr/local/bin/qnx-port-tests/test_silkit_adapters_tap    # its on-target test, from tests-8.0-<arch>-m-z
apk del silkit-adapters-tap qnx-test-silkit-adapters-tap
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/silkit-adapters-tap.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/silkit-adapters-tap.list
rm var/lib/manual/silkit-adapters-tap.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild silkit-adapters-tap` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild silkit
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.2.0 --recurse-submodules --shallow-submodules \
    https://github.com/vectorgrp/sil-kit-adapters-tap.git src/silkit-adapters-tap
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/silkit-adapters-tap -B build/$QNX_ARCH/silkit-adapters-tap -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DSILKIT_PACKAGE_DIR="$STAGE$PREFIX" \
      -DBUILD_LINUX_PACKAGE=OFF
cmake --build build/$QNX_ARCH/silkit-adapters-tap -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/silkit-adapters-tap
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Vector SIL Kit Adapter for TAP devices: connects a host network interface of type TAP to a SIL Kit Ethernet network, so that the host's own network stack (ping, sockets) talks to simulated Ethernet participants. Upstream supports QNX: on QNX it opens io-sock's /dev/tapN (`ifconfig tap0 create`), elsewhere Linux's /dev/net/tun. Built against the silkit component (SILKIT_PACKAGE_DIR: otherwise its CMake downloads a SIL Kit release); asio is the adapter's bundled copy (git submodule). Not built: the demos (a Linux shell-script ping demo and an ICMP echo device).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH silkit-adapters-tap`.

### 5. What it installs

1 files under `/usr/local`, 1 programs.

```
/usr/local/bin/sil-kit-adapter-tap
```

The complete list is `stage/<arch>/.manifest/silkit-adapters-tap` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa silkit-adapters-tap
ls dists/8.0/*/ | grep '^silkit-adapters-tap-'
```

This writes `silkit-adapters-tap-<ver>-r0.tar.gz` (run time) and `silkit-adapters-tap-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add silkit-adapters-tap
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/silkit-adapters-tap-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/silkit-adapters-tap-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_silkit_adapters_tap`](../../tests/src/) links silkit-adapters-tap the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-silkit-adapters-tap`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-silkit-adapters-tap
/usr/local/bin/qnx-port-tests/test_silkit_adapters_tap
```

```
== silkit-adapters-tap ==
  ok   ...
PASS silkit-adapters-tap
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_silkit_adapters_tap` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_silkit_adapters_tap`](../../tests/src/) -- a program linked against silkit-adapters-tap the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - temporary directory
   - --help lists --tap-name and --registry-uri
   - a missing TAP device is refused with the adapter's hint
   - the adapter opened /dev/tapN
   - tap interface brought up with the adapter attached
   - the host's ARP request for .2 arrives on the SIL Kit network
   - ARP request comes from the tap's address .1
   - after the ARP reply the UDP datagram is sent to the simulated MAC
   - UDP datagram to port 9 with its payload intact
   - a datagram injected on the SIL Kit side reaches a host socket
   - SIL Kit calls do not throw

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | silkit-adapters-tap 1.2.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/silkit-adapters-tap-1.2.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | silkit-adapters-tap 1.2.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/silkit-adapters-tap-1.2.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.2.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add silkit-adapters-tap=<version>`), sources in `ported/silkit-adapters-tap/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates silkit-adapters-tap`; port one with `scripts/port-update silkit-adapters-tap`.

<details><summary>recipe.sh</summary>

```bash
# Vector SIL Kit Adapter for TAP devices: connects a host network interface
# of type TAP to a SIL Kit Ethernet network, so that the host's own network
# stack (ping, sockets) talks to simulated Ethernet participants. Upstream
# supports QNX: on QNX it opens io-sock's /dev/tapN (`ifconfig tap0
# create`), elsewhere Linux's /dev/net/tun. Built against the silkit
# component (SILKIT_PACKAGE_DIR: otherwise its CMake downloads a SIL Kit
# release); asio is the adapter's bundled copy (git submodule). Not built:
# the demos (a Linux shell-script ping demo and an ICMP echo device).
VERSION=1.2.0; GIT=https://github.com/vectorgrp/sil-kit-adapters-tap.git; REF=v1.2.0
DEPS="silkit"
CMAKE_ARGS_EXTRA=( -DSILKIT_PACKAGE_DIR="$STAGE$PREFIX" -DBUILD_LINUX_PACKAGE=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
