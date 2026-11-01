# capicxx-core-runtime 3.2.4

> COVESA CommonAPI C++ core runtime for IPC interfaces

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/COVESA/capicxx-core-runtime.git (tag `3.2.4`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Built alone (empty stage) | aarch64le ✅ x86_64 ✅ |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_capicxx_core_runtime`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `capicxx-core-runtime` (programs, shared libraries, run-time data) and `capicxx-core-runtime-dev` (headers, pkg-config/CMake files, static libraries; needs `capicxx-core-runtime`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`capicxx-core-runtime`](https://openqnx.github.io/oss/package/x86_64/capicxx-core-runtime.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/capicxx-core-runtime.html)) · [`capicxx-core-runtime-dev`](https://openqnx.github.io/oss/package/x86_64/capicxx-core-runtime-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/capicxx-core-runtime-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add capicxx-core-runtime` (dependencies come along); one version: `apk add capicxx-core-runtime=<version>` | `apk del capicxx-core-runtime` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install capicxx-core-runtime` (dependencies first) | `oss-get remove capicxx-core-runtime` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`capicxx-core-runtime-dev` the same way, by its own name (`apk add capicxx-core-runtime-dev`, `oss-get install capicxx-core-runtime-dev`, by hand with its own file and list `capicxx-core-runtime-dev.list`); remove it before `capicxx-core-runtime`, which it needs.

```sh
# on the QNX target, as root
apk add capicxx-core-runtime capicxx-core-runtime-dev
apk add qnx-test-capicxx-core-runtime && /usr/local/bin/qnx-port-tests/test_capicxx_core_runtime    # its on-target test, from tests-8.0-<arch>
apk del capicxx-core-runtime capicxx-core-runtime-dev qnx-test-capicxx-core-runtime
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/capicxx-core-runtime.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/capicxx-core-runtime.list
rm var/lib/manual/capicxx-core-runtime.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild capicxx-core-runtime` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 3.2.4 --recurse-submodules --shallow-submodules \
    https://github.com/COVESA/capicxx-core-runtime.git src/capicxx-core-runtime
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/capicxx-core-runtime < ports/capicxx-core-runtime/patches/0001-qnx-no-use-after-free-at-exit.patch
```

**[`0001-qnx-no-use-after-free-at-exit.patch`](patches/0001-qnx-no-use-after-free-at-exit.patch)** — CommonAPI: do not reset the runtime from a static destructor.

> The deinitializer resets and deletes the runtime at exit, when static objects
> it still refers to may already be destroyed -- a use-after-free during exit.
> Leave the runtime to process teardown.

Touches: `src/CommonAPI/Runtime.cpp` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/capicxx-core-runtime -B build/$QNX_ARCH/capicxx-core-runtime -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/capicxx-core-runtime -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/capicxx-core-runtime
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH capicxx-core-runtime`.

### 5. What it installs

39 files under `/usr/local`: 2 shared-library entries, 32 headers.

```
/usr/local/lib/libCommonAPI.so
/usr/local/lib/libCommonAPI.so.3.2.4
```

The complete list is `stage/<arch>/.manifest/capicxx-core-runtime` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa capicxx-core-runtime
ls dists/8.0/*/ | grep '^capicxx-core-runtime-'
```

This writes `capicxx-core-runtime-<ver>-r0.tar.gz` (run time) and `capicxx-core-runtime-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add capicxx-core-runtime
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/capicxx-core-runtime-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/capicxx-core-runtime-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_capicxx_core_runtime`](../../tests/src/) links capicxx-core-runtime the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-capicxx-core-runtime`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-capicxx-core-runtime
/usr/local/bin/qnx-port-tests/test_capicxx_core_runtime
```

```
== capicxx-core-runtime ==
  ok   ...
PASS capicxx-core-runtime
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_capicxx_core_runtime` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
   Built again against an *empty* staging tree (`scripts/verify-standalone`): aarch64le passed, x86_64 passed -- it cannot have picked anything up from other components.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_capicxx_core_runtime`](../../tests/src/) -- a program linked against capicxx-core-runtime the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - Runtime::get returns one singleton
   - address split into domain / interface / instance
   - addresses compare equal
   - setInstance changes the address
   - Variant holds a string
   - Variant re-assigned to double
   - Variant equality
   - onFirstListenerAdded called once
   - both listeners notified in order
   - unsubscribed listener no longer called
   - onLastListenerRemoved called
   - Version
   - ByteBuffer
   - IniFileReader loads a file
   - ini values (whitespace trimmed)
   - second section
   - unknown section is null

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | capicxx-core-runtime 3.2.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/capicxx-core-runtime-3.2.4-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | capicxx-core-runtime 3.2.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/capicxx-core-runtime-3.2.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | capicxx-core-runtime 3.2.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/capicxx-core-runtime-3.2.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | capicxx-core-runtime 3.2.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/capicxx-core-runtime-3.2.4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.2.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add capicxx-core-runtime=<version>`), sources in `ported/capicxx-core-runtime/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates capicxx-core-runtime`; port one with `scripts/port-update capicxx-core-runtime`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.2.4; GIT=https://github.com/COVESA/capicxx-core-runtime.git; REF=3.2.4
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
