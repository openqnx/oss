# aws-c-auth 0.10.1

> C library for AWS client-side authentication: credentials and signing

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/awslabs/aws-c-auth.git (tag `v0.10.1`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`aws-c-http`](../aws-c-http/) [`aws-c-sdkutils`](../aws-c-sdkutils/) |
| Run-time dependencies | [`aws-c-cal`](../aws-c-cal/) [`aws-c-common`](../aws-c-common/) [`aws-c-compression`](../aws-c-compression/) [`aws-c-http`](../aws-c-http/) [`aws-c-io`](../aws-c-io/) [`aws-c-sdkutils`](../aws-c-sdkutils/) [`openssl`](../openssl/) [`s2n-tls`](../s2n-tls/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_aws_c_auth`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add aws-c-auth
apk add aws-c-auth-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-aws-c-auth && /usr/local/bin/qnx-port-tests/test_aws_c_auth    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install aws-c-auth`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild aws-c-auth` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (8 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild aws-c-http aws-c-sdkutils
```

<details><summary>Full build chain (8)</summary>

[`aws-c-cal`](../aws-c-cal/) [`aws-c-common`](../aws-c-common/) [`aws-c-compression`](../aws-c-compression/) [`aws-c-http`](../aws-c-http/) [`aws-c-io`](../aws-c-io/) [`aws-c-sdkutils`](../aws-c-sdkutils/) [`openssl`](../openssl/) [`s2n-tls`](../s2n-tls/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.10.1 --recurse-submodules --shallow-submodules \
    https://github.com/awslabs/aws-c-auth.git src/aws-c-auth
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/aws-c-auth -B build/$QNX_ARCH/aws-c-auth -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" \
      -DAWS_WARNINGS_ARE_ERRORS=OFF
cmake --build build/$QNX_ARCH/aws-c-auth -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/aws-c-auth
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The release aws-crt-cpp v0.38.5 pins as its submodule, so the whole AWS C runtime is one consistent set.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH aws-c-auth`.

### 5. What it installs

13 files under `/usr/local`: 2 shared-library entries, 8 headers.

```
/usr/local/lib/libaws-c-auth.so
/usr/local/lib/libaws-c-auth.so.1.0.0
```

The complete list is `stage/<arch>/.manifest/aws-c-auth` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa aws-c-auth
ls dists/8.0/*/ | grep '^aws-c-auth-'
```

This writes `aws-c-auth-<ver>-r0.tar.gz` (run time) and `aws-c-auth-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add aws-c-auth
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/aws-c-auth-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/aws-c-auth-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_aws_c_auth`](../../tests/src/) links aws-c-auth the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-aws-c-auth`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-aws-c-auth
/usr/local/bin/qnx-port-tests/test_aws_c_auth
```

```
== aws-c-auth ==
  ok   ...
PASS aws-c-auth
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_aws_c_auth` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_aws_c_auth`](../../tests/src/) -- a program linked against aws-c-auth the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - static credentials
   - SigV4 header signing completes
   - X-Amz-Date header added
   - SigV4 query-string signing completes
   - presigned query has algorithm, expiry and credential scope
   - presigned query carries a 64-hex-digit signature
   - static credentials provider
   - environment credentials provider

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aws-c-auth 0.10.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/aws-c-auth-0.10.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aws-c-auth 0.10.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/aws-c-auth-0.10.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aws-c-auth 0.10.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/aws-c-auth-0.10.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aws-c-auth 0.10.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/aws-c-auth-0.10.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.10.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add aws-c-auth=<version>`), sources in `ported/aws-c-auth/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates aws-c-auth`; port one with `scripts/port-update aws-c-auth`.

<details><summary>recipe.sh</summary>

```bash
# The release aws-crt-cpp v0.38.5 pins as its submodule, so the
# whole AWS C runtime is one consistent set.
VERSION=0.10.1; GIT=https://github.com/awslabs/aws-c-auth.git; REF=v0.10.1
DEPS="aws-c-http aws-c-sdkutils"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" -DAWS_WARNINGS_ARE_ERRORS=OFF  )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
