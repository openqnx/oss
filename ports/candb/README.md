# candb 0.1-git-20210625

> Library for parsing CAN bus database description formats

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/GENIVI/CANdb.git |
| Build system | CMake |
| QNX patches | 5 |
| Build dependencies | [`tl-expected`](../tl-expected/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_candb`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `candb` (programs, shared libraries, run-time data) and `candb-dev` (headers, pkg-config/CMake files, static libraries; needs `candb`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`candb`](https://openqnx.github.io/oss/package/x86_64/candb.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/candb.html)) · [`candb-dev`](https://openqnx.github.io/oss/package/x86_64/candb-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/candb-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add candb` (dependencies come along); one version: `apk add candb=<version>` | `apk del candb` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install candb` (dependencies first) | `oss-get remove candb` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`candb-dev` the same way, by its own name (`apk add candb-dev`, `oss-get install candb-dev`, by hand with its own file and list `candb-dev.list`); remove it before `candb`, which it needs.

```sh
# on the QNX target, as root
apk add candb candb-dev
apk add qnx-test-candb && /usr/local/bin/qnx-port-tests/test_candb    # its on-target test, from tests-8.0-<arch>
apk del candb candb-dev qnx-test-candb
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/candb.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/candb.list
rm var/lib/manual/candb.list
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

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild tl-expected
```

### 2. Get the upstream source

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/GENIVI/CANdb.git src/candb
```

### 3. Apply the QNX patches

5 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/candb < ports/candb/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/candb < ports/candb/patches/0002-bundled-gtest-only-for-tests.patch
patch -p1 -d src/candb < ports/candb/patches/0003-system-tl-expected.patch
patch -p1 -d src/candb < ports/candb/patches/0004-install-library-headers-tool.patch
patch -p1 -d src/candb < ports/candb/patches/0005-self-contained-library.patch
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

**[`0003-system-tl-expected.patch`](patches/0003-system-tl-expected.patch)** — Use an installed tl-expected when there is one.

> CANdb's API returns tl::expected (parser.hpp: CanDbOrError), so the
> tl/expected.hpp a program compiles against must be the one the library was
> built with. CANdb only knew its bundled copy (tl-expected 1.0.1), which is
> not installed; with tl-expected installed on the system (here the
> tl-expected 1.3.1 port), build against that one, so the installed headers
> (patch 0004) have a matching tl/expected.hpp. Without an installed
> tl-expected nothing changes.

Touches: `3rdParty/CMakeLists.txt` · Upstream status: Pending

**[`0004-install-library-headers-tool.patch`](patches/0004-install-library-headers-tool.patch)** — Install libCANdb, its API headers and dbclint.

> CANdb has no install rules: "make install" installed nothing of its own,
> only the bundled cxxopts' header and CMake package (which collide with the
> cxxopts port and are no longer installed, see recipe.sh). Install
> libCANdb (with a soname, libCANdb.so.0), the API headers dbcparser.h,
> parser.hpp, cantypes.hpp and parsererror.hpp as include/CANdb/, and the
> dbclint tool.

Touches: `src/CMakeLists.txt`, `tools/dbclint/CMakeLists.txt` · Upstream status: Pending

**[`0005-self-contained-library.patch`](patches/0005-self-contained-library.patch)** — Make libCANdb self-contained: a default logger, header-only spdlog/fmt.

> Two things kept the now installed libCANdb (patch 0004) from being usable by
> a program of its own:
>
> - The parser logs through kDefaultLogger (log.hpp), which libCANdb declares
>   but leaves to the program to define, as a std::shared_ptr to a logger of
>   CANdb's bundled spdlog 1.8.5 -- which is not installed, so a program could
>   not provide it. Define a default in the library: errors to stderr,
>   CDB_LEVEL to change the level (as dbclint does). It is weak, so a
>   program's own definition (dbclint's) still takes precedence.
> - log.hpp includes spdlog/fmt/ostr.h before spdlog.h, so the bundled fmt 7
>   was compiled in its library mode: extern templates (basic_data<void>,
>   vformat_to) that nothing in libCANdb defines, and linking a program
>   against it failed with undefined fmt::v7 symbols. (dbclint linked only
>   because it instantiates them itself.) Build CANdb with the bundled spdlog
>   and fmt header-only, as private definitions.

Touches: `src/CMakeLists.txt`, `src/default_logger.cpp` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/candb -B build/$QNX_ARCH/candb -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCANDB__WITH_TESTS=OFF \
      -DCANDB__WITH_TOOLS=ON \
      -DBUILD_SHARED_LIBS=ON \
      -DCXXOPTS_ENABLE_INSTALL=OFF
cmake --build build/$QNX_ARCH/candb -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/candb
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- tl-expected: CANdb's API returns tl::expected, so the library is built against the installed tl-expected (patch 0003) that its consumers use.
- Header API: consumers compile dbcparser.h, which includes tl/expected.hpp.
- Upstream installs nothing of its own; patch 0004 installs libCANdb, its API headers (include/CANdb/) and dbclint. dbclint is built with CANdb's bundled cxxopts 2.2, which is no longer installed (CXXOPTS_ENABLE_INSTALL =OFF): it put its header and CMake package where the cxxopts 3 port installs its own. dbclint is not built against that port: it uses the 2.x API (cxxopts::option_not_exists_exception, option names like "m, messages").

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH candb`.

### 5. What it installs

8 files under `/usr/local`: 3 shared-library entries, 1 programs, 4 headers.

```
/usr/local/lib/libCANdb.so
/usr/local/lib/libCANdb.so.0
/usr/local/lib/libCANdb.so.0.1
/usr/local/bin/dbclint
```

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

[`test_candb`](../../tests/src/) links candb the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-candb`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

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

   - DBC parsed
   - VERSION \"1.2.3\"
   - BU_: ECU1 ECU2 GATEWAY
   - two messages
   - BO_ 256 EngineData: 8 bytes from ECU1
   - EngineSpeed: 0|16@1+ (Intel, unsigned)
   - EngineSpeed: factor 0.25, range 0..16383.75 rpm
   - EngineSpeed receivers ECU2, GATEWAY
   - CoolantTemp: 16|8@1- (1,-40) degC
   - EngineData has two signals
   - BO_ 1024 BrakeStatus: one 1-bit Motorola signal
   - VAL_TABLE_ GEARS: 4 entries, 3 = DRIVE
   - a broken DBC is an error, not a database
   - empty input is an error

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
# tl-expected: CANdb's API returns tl::expected, so the library is built
# against the installed tl-expected (patch 0003) that its consumers use.
DEPS="tl-expected"
# Header API: consumers compile dbcparser.h, which includes tl/expected.hpp.
RUNTIME_DEPS="tl-expected-dev"
# Upstream installs nothing of its own; patch 0004 installs libCANdb, its
# API headers (include/CANdb/) and dbclint. dbclint is built with CANdb's
# bundled cxxopts 2.2, which is no longer installed (CXXOPTS_ENABLE_INSTALL
# =OFF): it put its header and CMake package where the cxxopts 3 port
# installs its own. dbclint is not built against that port: it uses the 2.x
# API (cxxopts::option_not_exists_exception, option names like "m, messages").
CMAKE_ARGS_EXTRA=( -DCANDB__WITH_TESTS=OFF -DCANDB__WITH_TOOLS=ON -DBUILD_SHARED_LIBS=ON -DCXXOPTS_ENABLE_INSTALL=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
