# zenoh-pico 1.2.1

> Eclipse Zenoh pub/sub, store/query and compute protocol for microcontrollers

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/eclipse-zenoh/zenoh-pico.git (tag `1.2.1`) |
| Build system | CMake |
| QNX patches | 3 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_zenoh_pico`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `zenoh-pico` (programs, shared libraries, run-time data) and `zenoh-pico-dev` (headers, pkg-config/CMake files, static libraries; needs `zenoh-pico`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`zenoh-pico`](https://openqnx.github.io/oss/package/x86_64/zenoh-pico.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/zenoh-pico.html)) · [`zenoh-pico-dev`](https://openqnx.github.io/oss/package/x86_64/zenoh-pico-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/zenoh-pico-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add zenoh-pico` (dependencies come along); one version: `apk add zenoh-pico=<version>` | `apk del zenoh-pico` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install zenoh-pico` (dependencies first) | `oss-get remove zenoh-pico` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`zenoh-pico-dev` the same way, by its own name (`apk add zenoh-pico-dev`, `oss-get install zenoh-pico-dev`, by hand with its own file and list `zenoh-pico-dev.list`); remove it before `zenoh-pico`, which it needs.

```sh
# on the QNX target, as root
apk add zenoh-pico zenoh-pico-dev
apk add qnx-test-zenoh-pico && /usr/local/bin/qnx-port-tests/test_zenoh_pico    # its on-target test, from tests-8.0-<arch>-m-z
apk del zenoh-pico zenoh-pico-dev qnx-test-zenoh-pico
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/zenoh-pico.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/zenoh-pico.list
rm var/lib/manual/zenoh-pico.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild zenoh-pico` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 1.2.1 --recurse-submodules --shallow-submodules \
    https://github.com/eclipse-zenoh/zenoh-pico.git src/zenoh-pico
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/zenoh-pico < ports/zenoh-pico/patches/0001-port-QNX7.1-8.0.patch
patch -p1 -d src/zenoh-pico < ports/zenoh-pico/patches/0002-fix-properly-excluding-raweth-for-non-linux-platform.patch
patch -p1 -d src/zenoh-pico < ports/zenoh-pico/patches/0003-qnx-urandom.patch
```

**[`0001-port-QNX7.1-8.0.patch`](patches/0001-port-QNX7.1-8.0.patch)** — port QNX7.1/8.0

> QNX as a zenoh-pico platform: the unix system layer (ZENOH_QNX), no -pipe
> (qcc rejects it), SO_NOSIGPIPE on TCP sockets as on the BSDs, and random
> numbers without Linux's getrandom() or the BSDs' arc4random().
>
>
> ---
>  CMakeLists.txt                              | 10 +++++--
>  include/zenoh-pico/system/common/platform.h |  2 +-
>  src/system/unix/network.c                   | 18 +++++++++++++
>  src/system/unix/system.c                    | 29 +++++++++++++++++++++
>  4 files changed, 56 insertions(+), 3 deletions(-)

Touches: `CMakeLists.txt`, `include/zenoh-pico/system/common/platform.h`, `src/system/unix/network.c`, `src/system/unix/system.c` · Upstream status: QNX-specific (from qnx-ports, github.com/qnx-ports/zenoh-pico branch qnx-1.2.1)

**[`0002-fix-properly-excluding-raweth-for-non-linux-platform.patch`](patches/0002-fix-properly-excluding-raweth-for-non-linux-platform.patch)** — fix properly excluding raweth for non-linux platforms

> raweth.c includes Linux-only headers (<linux/if_packet.h>) before its
> Z_FEATURE_RAWETH_TRANSPORT guard; move the guard above them so the file is
> empty on QNX, where the raw-ethernet transport is off.
>
>
> ---
>  src/system/unix/link/raweth.c | 4 ++--
>  1 file changed, 2 insertions(+), 2 deletions(-)

Touches: `src/system/unix/link/raweth.c` · Upstream status: QNX-specific (from qnx-ports, github.com/qnx-ports/zenoh-pico branch qnx-1.2.1)

**[`0003-qnx-urandom.patch`](patches/0003-qnx-urandom.patch)** — QNX: take random numbers from /dev/urandom, not unseeded random()

> qnx-ports' 0001 fills zenoh-pico's random numbers from random(), which nothing
> seeds: every process then draws the same sequence, so two zenoh-pico processes
> get the same session id (ZID) and the same request ids, and a router or peer
> treats them as one. QNX has no getrandom() (Linux) or arc4random() (BSD); it
> has /dev/urandom, which this reads (random() only if it cannot be opened). The
> helper is also static now -- 0001 exported a global "random_buf" symbol from
> libzenohpico.

Touches: `src/system/unix/system.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/zenoh-pico -B build/$QNX_ARCH/zenoh-pico -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_TOOLS=OFF \
      -DBUILD_INTEGRATION=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/zenoh-pico -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/zenoh-pico
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Eclipse Zenoh for constrained devices, in C. 1.2.1 is the release zenoh-c here is at (ports/zenoh-c), so the two speak the same protocol and zenoh-cpp can sit on either. patches/0001-0002 are qnx-ports' QNX support (branch qnx-1.2.1): QNX as a unix platform. 0003 replaces its random() calls with /dev/urandom. Examples, tools and the test programs are not installed; the shared library with the default feature set is (TCP and UDP unicast, UDP multicast and scouting; raw ethernet, serial and Bluetooth are off as upstream has them).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH zenoh-pico`.

### 5. What it installs

139 files under `/usr/local`: 1 shared-library entries, 133 headers.

```
/usr/local/lib/libzenohpico.so
```

The complete list is `stage/<arch>/.manifest/zenoh-pico` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa zenoh-pico
ls dists/8.0/*/ | grep '^zenoh-pico-'
```

This writes `zenoh-pico-<ver>-r0.tar.gz` (run time) and `zenoh-pico-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add zenoh-pico
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/zenoh-pico-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/zenoh-pico-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_zenoh_pico`](../../tests/src/) links zenoh-pico the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-zenoh-pico`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-zenoh-pico
/usr/local/bin/qnx-port-tests/test_zenoh_pico
```

```
== zenoh-pico ==
  ok   ...
PASS zenoh-pico
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_zenoh_pico` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_zenoh_pico`](../../tests/src/) -- a program linked against zenoh-pico the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - serialize/deserialize uint32 0xC0FFEE
   - a uint32 serializes to 4 bytes
   - open a second peer session on the same group
   - the two sessions have different random ids
   - declare a subscriber
   - the subscriber received a sample over UDP multicast
   - with the payload intact

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | zenoh-pico 1.2.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/zenoh-pico-1.2.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | zenoh-pico 1.2.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/zenoh-pico-1.2.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.2.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add zenoh-pico=<version>`), sources in `ported/zenoh-pico/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates zenoh-pico`; port one with `scripts/port-update zenoh-pico`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.2.1; GIT=https://github.com/eclipse-zenoh/zenoh-pico.git; REF=1.2.1
# Eclipse Zenoh for constrained devices, in C. 1.2.1 is the release zenoh-c
# here is at (ports/zenoh-c), so the two speak the same protocol and zenoh-cpp
# can sit on either. patches/0001-0002 are qnx-ports' QNX support (branch
# qnx-1.2.1): QNX as a unix platform. 0003 replaces its random() calls with
# /dev/urandom.
# Examples, tools and the test programs are not installed; the shared library
# with the default feature set is (TCP and UDP unicast, UDP multicast and
# scouting; raw ethernet, serial and Bluetooth are off as upstream has them).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_EXAMPLES=OFF -DBUILD_TOOLS=OFF
                   -DBUILD_INTEGRATION=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
