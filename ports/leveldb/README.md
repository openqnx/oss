# leveldb 1.23

> Fast ordered key-value storage library from Google

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/google/leveldb.git (tag `1.23`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`snappy`](../snappy/) |
| Run-time dependencies | [`snappy`](../snappy/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_leveldb`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add leveldb
apk add leveldb-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-leveldb && /usr/local/bin/qnx-port-tests/test_leveldb    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install leveldb`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild leveldb` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild snappy
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.23 --recurse-submodules --shallow-submodules \
    https://github.com/google/leveldb.git src/leveldb
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/leveldb -B build/$QNX_ARCH/leveldb -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DLEVELDB_BUILD_TESTS=OFF \
      -DLEVELDB_BUILD_BENCHMARKS=OFF
cmake --build build/$QNX_ARCH/leveldb -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/leveldb
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH leveldb`.

### 5. What it installs

22 files under `/usr/local`: 3 shared-library entries, 15 headers.

```
/usr/local/lib/libleveldb.so
/usr/local/lib/libleveldb.so.1
/usr/local/lib/libleveldb.so.1.23.0
```

The complete list is `stage/<arch>/.manifest/leveldb` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa leveldb
ls dists/8.0/*/ | grep '^leveldb-'
```

This writes `leveldb-<ver>-r0.tar.gz` (run time) and `leveldb-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add leveldb
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/leveldb-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/leveldb-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_leveldb`](../../tests/src/) links leveldb the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-leveldb`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-leveldb
/usr/local/bin/qnx-port-tests/test_leveldb
```

```
== leveldb ==
  ok   ...
PASS leveldb
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_leveldb` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_leveldb`](../../tests/src/) -- a program linked against leveldb the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - open
   - get
   - iterate

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | leveldb 1.23-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/leveldb-1.23-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | leveldb 1.23-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/leveldb-1.23-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.23**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add leveldb=<version>`), sources in `ported/leveldb/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates leveldb`; port one with `scripts/port-update leveldb`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.23; GIT=https://github.com/google/leveldb.git; REF=1.23
DEPS="snappy"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DLEVELDB_BUILD_TESTS=OFF -DLEVELDB_BUILD_BENCHMARKS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
