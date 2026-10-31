# aws-checksums 0.2.10

> Hardware-accelerated CRC32 and CRC32C checksum library for the AWS SDK

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/awslabs/aws-checksums.git (tag `v0.2.10`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`aws-c-common`](../aws-c-common/) |
| Run-time dependencies | [`aws-c-common`](../aws-c-common/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_aws_checksums`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add aws-checksums
apk add aws-checksums-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-aws-checksums && /usr/local/bin/qnx-port-tests/test_aws_checksums    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install aws-checksums`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild aws-checksums` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild aws-c-common
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.2.10 --recurse-submodules --shallow-submodules \
    https://github.com/awslabs/aws-checksums.git src/aws-checksums
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/aws-checksums -B build/$QNX_ARCH/aws-checksums -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" \
      -DAWS_WARNINGS_ARE_ERRORS=OFF
cmake --build build/$QNX_ARCH/aws-checksums -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/aws-checksums
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The release aws-crt-cpp v0.38.5 pins as its submodule, so the whole AWS C runtime is one consistent set.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH aws-checksums`.

### 5. What it installs

9 files under `/usr/local`: 2 shared-library entries, 4 headers.

```
/usr/local/lib/libaws-checksums.so
/usr/local/lib/libaws-checksums.so.1.0.0
```

The complete list is `stage/<arch>/.manifest/aws-checksums` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa aws-checksums
ls dists/8.0/*/ | grep '^aws-checksums-'
```

This writes `aws-checksums-<ver>-r0.tar.gz` (run time) and `aws-checksums-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add aws-checksums
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/aws-checksums-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/aws-checksums-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_aws_checksums`](../../tests/src/) links aws-checksums the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-aws-checksums`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-aws-checksums
/usr/local/bin/qnx-port-tests/test_aws_checksums
```

```
== aws-checksums ==
  ok   ...
PASS aws-checksums
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_aws_checksums` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_aws_checksums`](../../tests/src/) -- a program linked against aws-checksums the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - CRC32 check value
   - CRC32C check value
   - CRC64/NVME check value
   - CRC32 incremental
   - CRC32C combine
   - CRC64 combine
   - CRC32C of 1 MiB matches a bitwise reference
   - XXH64(\"abc\")
   - XXH3-64(\"\")
   - streaming XXH64

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aws-checksums 0.2.10-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/aws-checksums-0.2.10-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aws-checksums 0.2.10-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/aws-checksums-0.2.10-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.2.10**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add aws-checksums=<version>`), sources in `ported/aws-checksums/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates aws-checksums`; port one with `scripts/port-update aws-checksums`.

<details><summary>recipe.sh</summary>

```bash
# The release aws-crt-cpp v0.38.5 pins as its submodule, so the
# whole AWS C runtime is one consistent set.
VERSION=0.2.10; GIT=https://github.com/awslabs/aws-checksums.git; REF=v0.2.10
DEPS="aws-c-common"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" -DAWS_WARNINGS_ARE_ERRORS=OFF  )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
