# candb 0.1-git-20210625

> Library for parsing CAN bus database description formats

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/GENIVI/CANdb.git |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_candb`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `candb-dev` -- header-only: there is no run-time package, the -dev package is the whole component (headers, pkg-config/CMake files).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`candb-dev`](https://openqnx.github.io/oss/package/x86_64/candb-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/candb-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add candb-dev` (dependencies come along); one version: `apk add candb-dev=<version>` | `apk del candb-dev` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install candb-dev` (dependencies first) | `oss-get remove candb-dev` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-candb && /usr/local/bin/qnx-port-tests/test_candb    # its on-target test
apk del candb-dev qnx-test-candb
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/candb-dev.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/candb-dev.list
rm var/lib/manual/candb-dev.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild candb` performs exactly the same steps (2–5) in one command.

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
    https://github.com/GENIVI/CANdb.git src/candb
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/candb < ports/candb/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/candb < ports/candb/patches/0002-bundled-gtest-only-for-tests.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — add support for QNX 7.1/8.0

> Imported from qnx-ports/CANdb (150c42a8cdfc0748915f09d2cd0c5a68591263a7...qnx-master); original authorship kept.
>
>
> extend opendbc_tests + 8 additional dbc files
> ---
>  .gitignore                            |  3 ++
>  src/dbc_grammar.peg                   |  8 ++--
>  tests/CMakeLists.txt                  | 41 +++++++++++++++++-
>  tests/opendbc_tests.cpp               | 60 ++++++++++++++++++++++++++-
>  tests/opendbc_tests_expected_data.hpp |  2 +-
>  5 files changed, 107 insertions(+), 7 deletions(-)

Touches: `.gitignore`, `src/dbc_grammar.peg`, `tests/CMakeLists.txt`, `tests/opendbc_tests.cpp`, `tests/opendbc_tests_expected_data.hpp` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-bundled-gtest-only-for-tests.patch`](patches/0002-bundled-gtest-only-for-tests.patch)** — Build the bundled googletest only when tests are enabled.

> 3rdParty/CMakeLists.txt always builds the bundled (old) googletest and
> googlemock. With CANDB__WITH_TESTS=OFF nothing uses them, and in a tree that
> also has a current googletest installed the old gmock picks up the new gtest
> headers and fails to compile.

Touches: `3rdParty/CMakeLists.txt` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/candb -B build/$QNX_ARCH/candb -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCANDB__WITH_TESTS=OFF \
      -DCANDB__WITH_TOOLS=ON \
      -DBUILD_SHARED_LIBS=ON
cmake --build build/$QNX_ARCH/candb -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/candb
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH candb`.

### 5. What it installs

4 files under `/usr/local`, 1 headers.

The complete list is `stage/<arch>/.manifest/candb` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa candb
ls dists/8.0/*/ | grep '^candb-'
```

This writes `candb-<ver>-r0.tar.gz` (run time) and `candb-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add candb
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/candb-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/candb-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_candb`](../../tests/src/) links candb the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-candb`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-candb
/usr/local/bin/qnx-port-tests/test_candb
```

```
== candb ==
  ok   ...
PASS candb
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_candb` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_candb`](../../tests/src/) -- a program linked against candb the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - -n 7 -> count 7
   - bitrate default 500000, not given
   - --name=can0
   - -v -v counted twice
   - --id repeated: 256, 512
   - positionals a.dbc b.dbc
   - help text lists --count
   - valid command line parsed
   - unknown option --nope rejected
   - -n seven rejected as not an int

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | candb 0.1_git20210625-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/candb-0.1_git20210625-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | candb 0.1_git20210625-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/candb-0.1_git20210625-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | candb 0.1_git20210625-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/candb-0.1_git20210625-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | candb 0.1_git20210625-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/candb-0.1_git20210625-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.1-git-20210625**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add candb=<version>`), sources in `ported/candb/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates candb`; port one with `scripts/port-update candb`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.1-git-20210625; GIT=https://github.com/GENIVI/CANdb.git; COMMIT=150c42a8cdfc0748915f09d2cd0c5a68591263a7
CMAKE_ARGS_EXTRA=( -DCANDB__WITH_TESTS=OFF -DCANDB__WITH_TOOLS=ON -DBUILD_SHARED_LIBS=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
