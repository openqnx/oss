# ntttcp-for-linux 1.4.0-git-20230613

> Multi-threaded Linux network throughput benchmark (NTTTCP for Linux)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/microsoft/ntttcp-for-linux.git |
| Build system | CMake |
| QNX patches | 4 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_ntttcp_for_linux`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `ntttcp-for-linux`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`ntttcp-for-linux`](https://openqnx.github.io/oss/package/x86_64/ntttcp-for-linux.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ntttcp-for-linux.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add ntttcp-for-linux` (dependencies come along); one version: `apk add ntttcp-for-linux=<version>` | `apk del ntttcp-for-linux` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install ntttcp-for-linux` (dependencies first) | `oss-get remove ntttcp-for-linux` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-ntttcp-for-linux && /usr/local/bin/qnx-port-tests/test_ntttcp_for_linux    # its on-target test, from tests-8.0-<arch>-m-z
apk del ntttcp-for-linux qnx-test-ntttcp-for-linux
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/ntttcp-for-linux.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/ntttcp-for-linux.list
rm var/lib/manual/ntttcp-for-linux.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ntttcp-for-linux` performs exactly the same steps (2–5) in one command.

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
    https://github.com/microsoft/ntttcp-for-linux.git src/ntttcp-for-linux
```

The project lives in the `src/` subdirectory of the repository.

### 3. Apply the QNX patches

4 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/ntttcp-for-linux < ports/ntttcp-for-linux/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/ntttcp-for-linux < ports/ntttcp-for-linux/patches/0002-qnx-runmask.patch
patch -p1 -d src/ntttcp-for-linux < ports/ntttcp-for-linux/patches/0003-receiver-wait-timeout.patch
patch -p1 -d src/ntttcp-for-linux < ports/ntttcp-for-linux/patches/0004-qnx-no-linux-proc.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — ntttcp: QNX build support for till CI-b483afe

> Imported from qnx-ports/ntttcp-for-linux (b483afe00b9b...37f18bdb1552, branch qnx-b483afe); original authorship kept.
>
> ---
>  src/CMakeLists.txt |  7 ++++++-
>  src/main.c         | 11 +++++++++++
>  src/tcpstream.c    |  3 +++
>  3 files changed, 20 insertions(+), 1 deletion(-)

Touches: `src/CMakeLists.txt`, `src/main.c`, `src/tcpstream.c` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-runmask.patch`](patches/0002-qnx-runmask.patch)** — Set CPU affinity on QNX with a proper ThreadCtl() run mask.

> qnx-ports' QNX change (0001) passes the CPU number from -m as the run mask
> of ThreadCtl(_NTO_TCTL_RUNMASK), by address, with _NTO_TCTL_RUNMASK
> hard-coded. The run mask is a bit mask of CPUs given as the pointer value
> itself: "-m 1,*,..." asked for nothing sensible and the call failed
> (ntttcp then exited). Take ThreadCtl() and the constant from
> <sys/neutrino.h>, pass 1 << cpu, and report a failure the way the Linux
> path does (an error message, the test goes on). Also drops the cpu_set_t
> variable that only the Linux path uses (an unused-variable warning).

Touches: `src/main.c` · Upstream status: QNX-specific

**[`0003-receiver-wait-timeout.patch`](patches/0003-receiver-wait-timeout.patch)** — Let the TCP receiver threads notice the end of a test.

> The TCP receiver threads leave their accept/receive loop only when they
> find the test finished at the top of the loop, and then block in select()
> or epoll_wait() with no timeout. When the sender's connections close before
> the receiver's own test timer has marked the test finished -- the usual
> order on QNX, where both sides' timers expire within the same
> millisecond -- the thread handles the EOFs, goes back to waiting on the
> listening socket alone and never wakes again: the receiver printed its
> results and then hung for good in pthread_join(). Wait at most one second,
> so the loop sees the finished state. (The UDP receiver already uses a
> receive timeout for the same reason.)

Touches: `src/tcpstream.c` · Upstream status: Pending

**[`0004-qnx-no-linux-proc.patch`](patches/0004-qnx-no-linux-proc.patch)** — Do not report Linux-only /proc files as errors on QNX.

> ntttcp samples Linux's /proc/stat, /proc/net/snmp, /proc/net/netstat,
> /proc/interrupts and /proc/cpuinfo before and after every test. QNX has
> none of them, so every run printed a dozen "ERR : failed to open file"
> lines although the test itself was fine, and reported "cpu cores: 0".
> On QNX a missing file is now not an error (its counters stay 0, as
> before), and the CPU count comes from sysconf(_SC_NPROCESSORS_ONLN).

Touches: `src/oscounter.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/ntttcp-for-linux/src -B build/$QNX_ARCH/ntttcp-for-linux -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF
cmake --build build/$QNX_ARCH/ntttcp-for-linux -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/ntttcp-for-linux
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- NTTTCP for Linux at commit b483afe (2023-06-13, tool version 1.4.0): the upstream commit qnx-ports/ntttcp-for-linux (branch qnx-b483afe) ports.
- The CMake project is in src/ (the top level has only docs and scripts).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ntttcp-for-linux`.

### 5. What it installs

1 files under `/usr/local`, 1 programs.

```
/usr/local/bin/ntttcp
```

The complete list is `stage/<arch>/.manifest/ntttcp-for-linux` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ntttcp-for-linux
ls dists/8.0/*/ | grep '^ntttcp-for-linux-'
```

This writes `ntttcp-for-linux-<ver>-r0.tar.gz` (run time) and `ntttcp-for-linux-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ntttcp-for-linux
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ntttcp-for-linux-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ntttcp-for-linux-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_ntttcp_for_linux`](../../tests/src/) links ntttcp-for-linux the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-ntttcp-for-linux`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-ntttcp-for-linux
/usr/local/bin/qnx-port-tests/test_ntttcp_for_linux
```

```
== ntttcp-for-linux ==
  ok   ...
PASS ntttcp-for-linux
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_ntttcp_for_linux` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_ntttcp_for_linux`](../../tests/src/) -- a program linked against ntttcp-for-linux the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ntttcp-for-linux 1.4.0_git20230613-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/ntttcp-for-linux-1.4.0_git20230613-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ntttcp-for-linux 1.4.0_git20230613-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/ntttcp-for-linux-1.4.0_git20230613-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.4.0-git-20230613**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ntttcp-for-linux=<version>`), sources in `ported/ntttcp-for-linux/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ntttcp-for-linux`; port one with `scripts/port-update ntttcp-for-linux`.

<details><summary>recipe.sh</summary>

```bash
# NTTTCP for Linux at commit b483afe (2023-06-13, tool version 1.4.0): the
# upstream commit qnx-ports/ntttcp-for-linux (branch qnx-b483afe) ports.
VERSION=1.4.0-git-20230613; GIT=https://github.com/microsoft/ntttcp-for-linux.git; COMMIT=b483afe00b9b095e8bf56c01308d81aeae0cb53c
# The CMake project is in src/ (the top level has only docs and scripts).
SUBDIR=src
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
