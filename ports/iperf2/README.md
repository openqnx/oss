# iperf2 2.2.1

> Tool for measuring network bandwidth, TCP and UDP performance (iperf 2)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/project/iperf2/iperf-2.2.1.tar.gz |
| SHA-256 | `754ab0a7e28033dbea81308ef424bc7df4d6e2fe31b60cc536b61b51fefbd8fb` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_iperf2`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `iperf2` (programs, shared libraries, run-time data) and `iperf2-dev` (headers, pkg-config/CMake files, static libraries; needs `iperf2`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`iperf2`](https://openqnx.github.io/oss/package/x86_64/iperf2.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/iperf2.html)) · [`iperf2-dev`](https://openqnx.github.io/oss/package/x86_64/iperf2-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/iperf2-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add iperf2` (dependencies come along); one version: `apk add iperf2=<version>` | `apk del iperf2` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install iperf2` (dependencies first) | `oss-get remove iperf2` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`iperf2-dev` the same way, by its own name (`apk add iperf2-dev`, `oss-get install iperf2-dev`, by hand with its own file and list `iperf2-dev.list`); remove it before `iperf2`, which it needs.

```sh
# on the QNX target, as root
apk add iperf2 iperf2-dev
apk add qnx-test-iperf2 && /usr/local/bin/qnx-port-tests/test_iperf2    # its on-target test, from tests-8.0-<arch>
apk del iperf2 iperf2-dev qnx-test-iperf2
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/iperf2.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/iperf2.list
rm var/lib/manual/iperf2.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild iperf2` performs exactly the same steps (2–5) in one command.

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
URL=https://downloads.sourceforge.net/project/iperf2/iperf-2.2.1.tar.gz
mkdir -p dist src/iperf2
curl -fL -o dist/$(basename $URL) $URL
echo "754ab0a7e28033dbea81308ef424bc7df4d6e2fe31b60cc536b61b51fefbd8fb  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/iperf2 --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/iperf2 < ports/iperf2/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — add support for QNX 7.1/8.0 with recv-mmsg/send-mmsg

> Imported from qnx-ports/iperf2 (5fe98129177e...d4954fb61f7e, branch qnx-2-2-1); original authorship kept.
>
> ---
>  include/Client.hpp   |   6 +-
>  include/Locale.h     |  10 +++
>  include/Reporter.h   |   7 ++
>  include/Server.hpp   |  18 +++-
>  include/Settings.hpp |  47 ++++++++++
>  include/payloads.h   |   4 +
>  src/Client.cpp       | 200 +++++++++++++++++++++++++++++++++++++++++--
>  src/Listener.cpp     |   7 ++
>  src/Locale.c         |  31 ++++++-
>  src/ReportOutputs.c  |  59 +++++++++----
>  src/Reports.c        |   7 ++
>  src/Server.cpp       | 127 ++++++++++++++++++++++++++-
>  src/Settings.cpp     |  81 ++++++++++++++++++
>  src/checksums.c      |   4 +
>  src/main.cpp         |   5 ++
>  15 files changed, 579 insertions(+), 34 deletions(-)

Touches: `include/Client.hpp`, `include/Locale.h`, `include/Reporter.h`, `include/Server.hpp`, `include/Settings.hpp`, `include/payloads.h` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/iperf2 && cd build/$QNX_ARCH/iperf2
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/iperf2/configure --host=$QNX_TRIPLE --prefix=/usr/local
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- iperf 2 (the iperf2 project on SourceForge, not esnet's iperf3; installs bin/iperf). The QNX support is qnx-ports/iperf2 (branch qnx-2-2-1, based on upstream's 2-2-1 branch = this release): QNX 7.1/8.0 cases for the recvmmsg()/sendmmsg() batched UDP paths and the headers they need.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH iperf2`.

### 5. What it installs

2 files under `/usr/local`, 1 programs.

```
/usr/local/bin/iperf
```

The complete list is `stage/<arch>/.manifest/iperf2` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa iperf2
ls dists/8.0/*/ | grep '^iperf2-'
```

This writes `iperf2-<ver>-r0.tar.gz` (run time) and `iperf2-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add iperf2
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/iperf2-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/iperf2-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_iperf2`](../../tests/src/) links iperf2 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-iperf2`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-iperf2
/usr/local/bin/qnx-port-tests/test_iperf2
```

```
== iperf2 ==
  ok   ...
PASS iperf2
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_iperf2` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_iperf2`](../../tests/src/) -- a program linked against iperf2 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | iperf2 2.2.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/iperf2-2.2.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | iperf2 2.2.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/iperf2-2.2.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.2.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add iperf2=<version>`), sources in `ported/iperf2/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates iperf2`; port one with `scripts/port-update iperf2`.

<details><summary>recipe.sh</summary>

```bash
# iperf 2 (the iperf2 project on SourceForge, not esnet's iperf3; installs
# bin/iperf). The QNX support is qnx-ports/iperf2 (branch qnx-2-2-1, based on
# upstream's 2-2-1 branch = this release): QNX 7.1/8.0 cases for the
# recvmmsg()/sendmmsg() batched UDP paths and the headers they need.
VERSION=2.2.1
TARBALL=https://downloads.sourceforge.net/project/iperf2/iperf-2.2.1.tar.gz
SHA256=754ab0a7e28033dbea81308ef424bc7df4d6e2fe31b60cc536b61b51fefbd8fb
BUILDSYS=autotools
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
