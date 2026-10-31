# aws-c-mqtt 0.15.2

> C implementation of the MQTT 3.1.1 and MQTT 5 protocols for AWS

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/awslabs/aws-c-mqtt.git (tag `v0.15.2`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`aws-c-http`](../aws-c-http/) |
| Run-time dependencies | [`aws-c-cal`](../aws-c-cal/) [`aws-c-common`](../aws-c-common/) [`aws-c-compression`](../aws-c-compression/) [`aws-c-http`](../aws-c-http/) [`aws-c-io`](../aws-c-io/) [`openssl`](../openssl/) [`s2n-tls`](../s2n-tls/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_aws_c_mqtt`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add aws-c-mqtt
apk add aws-c-mqtt-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-aws-c-mqtt && /usr/local/bin/qnx-port-tests/test_aws_c_mqtt    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install aws-c-mqtt`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild aws-c-mqtt` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (7 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild aws-c-http
```

<details><summary>Full build chain (7)</summary>

[`aws-c-cal`](../aws-c-cal/) [`aws-c-common`](../aws-c-common/) [`aws-c-compression`](../aws-c-compression/) [`aws-c-http`](../aws-c-http/) [`aws-c-io`](../aws-c-io/) [`openssl`](../openssl/) [`s2n-tls`](../s2n-tls/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.15.2 --recurse-submodules --shallow-submodules \
    https://github.com/awslabs/aws-c-mqtt.git src/aws-c-mqtt
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/aws-c-mqtt -B build/$QNX_ARCH/aws-c-mqtt -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" \
      -DAWS_WARNINGS_ARE_ERRORS=OFF
cmake --build build/$QNX_ARCH/aws-c-mqtt -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/aws-c-mqtt
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The release aws-crt-cpp v0.38.5 pins as its submodule, so the whole AWS C runtime is one consistent set.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH aws-c-mqtt`.

### 5. What it installs

14 files under `/usr/local`: 2 shared-library entries, 9 headers.

```
/usr/local/lib/libaws-c-mqtt.so
/usr/local/lib/libaws-c-mqtt.so.1.0.0
```

The complete list is `stage/<arch>/.manifest/aws-c-mqtt` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa aws-c-mqtt
ls dists/8.0/*/ | grep '^aws-c-mqtt-'
```

This writes `aws-c-mqtt-<ver>-r0.tar.gz` (run time) and `aws-c-mqtt-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add aws-c-mqtt
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/aws-c-mqtt-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/aws-c-mqtt-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_aws_c_mqtt`](../../tests/src/) links aws-c-mqtt the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-aws-c-mqtt`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-aws-c-mqtt
/usr/local/bin/qnx-port-tests/test_aws_c_mqtt
```

```
== aws-c-mqtt ==
  ok   ...
PASS aws-c-mqtt
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_aws_c_mqtt` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_aws_c_mqtt`](../../tests/src/) -- a program linked against aws-c-mqtt the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - test broker listening on 127.0.0.1
   - MQTT client and connection objects
   - connect started
   - CONNACK accepted
   - broker saw CONNECT: MQTT 3.1.1, clean session, client id, keep-alive 30
   - subscribe queued
   - SUBACK grants QoS 1
   - broker saw SUBSCRIBE qnx/test/# at QoS 1
   - publish queued
   - PUBACK completes the QoS 1 publish
   - broker saw PUBLISH with topic and payload
   - echoed message delivered to the subscription
   - delivered topic and payload
   - disconnect completes
   - broker saw DISCONNECT

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aws-c-mqtt 0.15.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/aws-c-mqtt-0.15.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aws-c-mqtt 0.15.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/aws-c-mqtt-0.15.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aws-c-mqtt 0.15.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/aws-c-mqtt-0.15.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aws-c-mqtt 0.15.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/aws-c-mqtt-0.15.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.15.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add aws-c-mqtt=<version>`), sources in `ported/aws-c-mqtt/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates aws-c-mqtt`; port one with `scripts/port-update aws-c-mqtt`.

<details><summary>recipe.sh</summary>

```bash
# The release aws-crt-cpp v0.38.5 pins as its submodule, so the
# whole AWS C runtime is one consistent set.
VERSION=0.15.2; GIT=https://github.com/awslabs/aws-c-mqtt.git; REF=v0.15.2
DEPS="aws-c-http"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" -DAWS_WARNINGS_ARE_ERRORS=OFF  )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
