# check 0.15.2

> Unit testing framework for C

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/libcheck/check.git (tag `0.15.2`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_check`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add check
apk add check-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-check && /usr/local/bin/qnx-port-tests/test_check    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install check`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild check` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 0.15.2 --recurse-submodules --shallow-submodules \
    https://github.com/libcheck/check.git src/check
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/check -B build/$QNX_ARCH/check -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCHECK_ENABLE_TESTS=OFF \
      -DCHECK_ENABLE_TIMEOUT_TESTS=OFF \
      -DAWK_PATH=/usr/local/bin/awk
cmake --build build/$QNX_ARCH/check -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/check
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- checkmk is an awk script whose "#!" line is configured from find_program(awk) -- on the build machine, so it named the cross build's linkroot awk (/home/.../stage/x86_64/linkroot/x86_64/usr/bin/awk), which no target has. Point it at the awk the gawk package installs on the target, and depend on it.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH check`.

### 5. What it installs

13 files under `/usr/local`: 3 shared-library entries, 1 programs, 2 headers.

```
/usr/local/lib/libcheck.so
/usr/local/lib/libcheck.so.0
/usr/local/lib/libcheck.so.0.15.2
/usr/local/bin/checkmk
```

The complete list is `stage/<arch>/.manifest/check` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa check
ls dists/8.0/*/ | grep '^check-'
```

This writes `check-<ver>-r0.tar.gz` (run time) and `check-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add check
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/check-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/check-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_check`](../../tests/src/) links check the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-check`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-check
/usr/local/bin/qnx-port-tests/test_check
```

```
== check ==
  ok   ...
PASS check
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_check` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_check`](../../tests/src/) -- a program linked against check the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - library version 0.15.2
   - fork mode: 9 tests run
   - fork mode: 2 not passed (deliberate failure + abort)
   - fork mode: 7 pass, 1 failure, 1 error
   - failure message is \"deliberate failure 42\"
   - abort() reported as an error by signal
   - no-fork mode: 6 run, 1 failed
   - checkmk's #! interpreter exists on the target
   - mkdtemp
   - checkmk generated suite, two START_TESTs, tcase_add_test and srunner_run_all

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | check 0.15.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/check-0.15.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | check 0.15.2-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/check-0.15.2-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | check 0.15.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/check-0.15.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | check 0.15.2-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/check-0.15.2-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.15.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add check=<version>`), sources in `ported/check/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates check`; port one with `scripts/port-update check`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.15.2; GIT=https://github.com/libcheck/check.git; REF=0.15.2
# checkmk is an awk script whose "#!" line is configured from find_program(awk)
# -- on the build machine, so it named the cross build's linkroot awk
# (/home/.../stage/x86_64/linkroot/x86_64/usr/bin/awk), which no target has.
# Point it at the awk the gawk package installs on the target, and depend on it.
RUNTIME_DEPS="gawk"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCHECK_ENABLE_TESTS=OFF -DCHECK_ENABLE_TIMEOUT_TESTS=OFF
                   -DAWK_PATH=/usr/local/bin/awk )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
