# dlt-daemon 2.18.10

> COVESA Diagnostic Log and Trace (DLT) daemon, library and tools

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/COVESA/dlt-daemon.git (tag `v2.18.10`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`zlib`](../zlib/) [`libjson`](../libjson/) |
| Run-time dependencies | [`libjson`](../libjson/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_dlt_daemon`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `dlt-daemon` (programs, shared libraries, run-time data) and `dlt-daemon-dev` (headers, pkg-config/CMake files, static libraries; needs `dlt-daemon`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`dlt-daemon`](https://openqnx.github.io/oss/package/x86_64/dlt-daemon.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/dlt-daemon.html)) · [`dlt-daemon-dev`](https://openqnx.github.io/oss/package/x86_64/dlt-daemon-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/dlt-daemon-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add dlt-daemon` (dependencies come along); one version: `apk add dlt-daemon=<version>` | `apk del dlt-daemon` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install dlt-daemon` (dependencies first) | `oss-get remove dlt-daemon` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`dlt-daemon-dev` the same way, by its own name (`apk add dlt-daemon-dev`, `oss-get install dlt-daemon-dev`, by hand with its own file and list `dlt-daemon-dev.list`); remove it before `dlt-daemon`, which it needs.

```sh
# on the QNX target, as root
apk add dlt-daemon dlt-daemon-dev
apk add qnx-test-dlt-daemon && /usr/local/bin/qnx-port-tests/test_dlt_daemon    # its on-target test
apk del dlt-daemon dlt-daemon-dev qnx-test-dlt-daemon
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/dlt-daemon.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/dlt-daemon.list
rm var/lib/manual/dlt-daemon.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild dlt-daemon` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (2 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib libjson
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v2.18.10 --recurse-submodules --shallow-submodules \
    https://github.com/COVESA/dlt-daemon.git src/dlt-daemon
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/dlt-daemon < ports/dlt-daemon/patches/0001-qnx-syslog-header.patch
```

**[`0001-qnx-syslog-header.patch`](patches/0001-qnx-syslog-header.patch)** — DLT: include <syslog.h> on QNX.

> <sys/syslog.h> is a glibc/BSD alias that QNX does not ship; QNX has the POSIX
> <syslog.h>.

Touches: `src/offlinelogstorage/dlt_offline_logstorage.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/dlt-daemon -B build/$QNX_ARCH/dlt-daemon -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DWITH_DLT_QNX_SYSTEM=ON \
      -DWITH_DLT_CXX11_EXT=ON \
      -DDLT_IPC=UNIX_SOCKET \
      -DWITH_DLT_ADAPTOR=ON \
      -DWITH_DLT_USE_IPv6=OFF \
      -DWITH_LIB_SHORT_VERSION=ON \
      -DWITH_SYSTEMD=OFF \
      -DWITH_DLT_UNIT_TESTS=OFF \
      -DWITH_DLT_INSTALLED_TESTS=OFF \
      -DWITH_DLT_EXAMPLES=OFF \
      # \
      upstream \
      adds \
      dlt-test-cpp-extension \
      twice \
      when \
      CXX11_EXT \
      is \
      on \
      -DWITH_DLT_TESTS=OFF
cmake --build build/$QNX_ARCH/dlt-daemon -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/dlt-daemon
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): libjson

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH dlt-daemon`.

### 5. What it installs

35 files under `/usr/local`: 2 shared-library entries, 10 programs, 14 headers.

```
/usr/local/lib/libdlt.so
/usr/local/lib/libdlt.so.2
/usr/local/bin/dlt-adaptor-stdin
/usr/local/bin/dlt-adaptor-udp
/usr/local/bin/dlt-control
/usr/local/bin/dlt-convert
/usr/local/bin/dlt-daemon
/usr/local/bin/dlt-logstorage-ctrl
/usr/local/bin/dlt-passive-node-ctrl
/usr/local/bin/dlt-qnx-system
/usr/local/bin/dlt-receive
/usr/local/bin/dlt-sortbytimestamp
```

The complete list is `stage/<arch>/.manifest/dlt-daemon` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa dlt-daemon
ls dists/8.0/*/ | grep '^dlt-daemon-'
```

This writes `dlt-daemon-<ver>-r0.tar.gz` (run time) and `dlt-daemon-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add dlt-daemon
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/dlt-daemon-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/dlt-daemon-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_dlt_daemon`](../../tests/src/) links dlt-daemon the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-dlt-daemon`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-dlt-daemon
/usr/local/bin/qnx-port-tests/test_dlt_daemon
```

```
== dlt-daemon ==
  ok   ...
PASS dlt-daemon
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_dlt_daemon` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_dlt_daemon`](../../tests/src/) -- a program linked against dlt-daemon the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - scratch directory
   - dlt-daemon listens on 127.0.0.1 and created /tmp/dlt
   - five messages logged through libdlt
   - dlt_file_open on the received file
   - each of qnx-dlt-msg 0..4 received exactly once
   - app ID QNXT / context ID TCTX on our messages
   - dlt-convert -a prints \"... QNXT TCTX log info ... qnx-dlt-msg 3\"
   - dlt-convert -c counts the messages

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | dlt-daemon 2.18.10-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/dlt-daemon-2.18.10-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | dlt-daemon 2.18.10-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/dlt-daemon-2.18.10-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | dlt-daemon 2.18.10-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/dlt-daemon-2.18.10-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | dlt-daemon 2.18.10-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/dlt-daemon-2.18.10-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.18.10**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add dlt-daemon=<version>`), sources in `ported/dlt-daemon/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates dlt-daemon`; port one with `scripts/port-update dlt-daemon`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.18.10; GIT=https://github.com/COVESA/dlt-daemon.git; REF=v2.18.10
# Also linked at run time (found by scripts/gen-dependency-index): libjson
DEPS="zlib libjson"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DWITH_DLT_QNX_SYSTEM=ON
                   -DWITH_DLT_CXX11_EXT=ON -DDLT_IPC=UNIX_SOCKET
                   -DWITH_DLT_ADAPTOR=ON -DWITH_DLT_USE_IPv6=OFF
                   -DWITH_LIB_SHORT_VERSION=ON -DWITH_SYSTEMD=OFF
                   -DWITH_DLT_UNIT_TESTS=OFF -DWITH_DLT_INSTALLED_TESTS=OFF
                   -DWITH_DLT_EXAMPLES=OFF
                   # upstream adds dlt-test-cpp-extension twice when CXX11_EXT is on
                   -DWITH_DLT_TESTS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
