# aws-crt-cpp 0.38.5

> C++ wrapper around the AWS Common Runtime (aws-c-*) libraries

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/awslabs/aws-crt-cpp.git (tag `v0.38.5`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`aws-c-auth`](../aws-c-auth/) [`aws-c-mqtt`](../aws-c-mqtt/) [`aws-c-event-stream`](../aws-c-event-stream/) [`aws-c-s3`](../aws-c-s3/) |
| Run-time dependencies | [`aws-c-auth`](../aws-c-auth/) [`aws-c-cal`](../aws-c-cal/) [`aws-c-common`](../aws-c-common/) [`aws-c-compression`](../aws-c-compression/) [`aws-c-event-stream`](../aws-c-event-stream/) [`aws-c-http`](../aws-c-http/) [`aws-c-io`](../aws-c-io/) [`aws-c-mqtt`](../aws-c-mqtt/) [`aws-c-s3`](../aws-c-s3/) [`aws-c-sdkutils`](../aws-c-sdkutils/) [`aws-checksums`](../aws-checksums/) [`openssl`](../openssl/) [`s2n-tls`](../s2n-tls/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_aws_crt_cpp`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add aws-crt-cpp
apk add aws-crt-cpp-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-aws-crt-cpp && /usr/local/bin/qnx-port-tests/test_aws_crt_cpp    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install aws-crt-cpp`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild aws-crt-cpp` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (13 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild aws-c-auth aws-c-mqtt aws-c-event-stream aws-c-s3
```

<details><summary>Full build chain (13)</summary>

[`aws-c-auth`](../aws-c-auth/) [`aws-c-cal`](../aws-c-cal/) [`aws-c-common`](../aws-c-common/) [`aws-c-compression`](../aws-c-compression/) [`aws-c-event-stream`](../aws-c-event-stream/) [`aws-c-http`](../aws-c-http/) [`aws-c-io`](../aws-c-io/) [`aws-c-mqtt`](../aws-c-mqtt/) [`aws-c-s3`](../aws-c-s3/) [`aws-c-sdkutils`](../aws-c-sdkutils/) [`aws-checksums`](../aws-checksums/) [`openssl`](../openssl/) [`s2n-tls`](../s2n-tls/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.38.5 --recurse-submodules --shallow-submodules \
    https://github.com/awslabs/aws-crt-cpp.git src/aws-crt-cpp
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/aws-crt-cpp < ports/aws-crt-cpp/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add QNX support

> Imported from qnx-ports/aws-crt-cpp (v0.30.1...qnx-0.30.1).
>
> ---
>  CMakeLists.txt                    | 10 +++++++++-
>  bin/elasticurl_cpp/CMakeLists.txt |  2 ++
>  bin/mqtt5_app/CMakeLists.txt      |  2 ++
>  bin/mqtt5_canary/CMakeLists.txt   |  2 ++
>  4 files changed, 15 insertions(+), 1 deletion(-)

Touches: `CMakeLists.txt`, `bin/elasticurl_cpp/CMakeLists.txt`, `bin/mqtt5_app/CMakeLists.txt`, `bin/mqtt5_canary/CMakeLists.txt` · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/aws-crt-cpp -B build/$QNX_ARCH/aws-crt-cpp -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_DEPS=OFF \
      -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" \
      -DAWS_WARNINGS_ARE_ERRORS=OFF
cmake --build build/$QNX_ARCH/aws-crt-cpp -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/aws-crt-cpp
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- BUILD_DEPS=OFF: build against the separately packaged aws-c-* components instead of the copies in crt/ (whose submodules are therefore not needed).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH aws-crt-cpp`.

### 5. What it installs

58 files under `/usr/local`: 1 shared-library entries, 53 headers.

```
/usr/local/lib/libaws-crt-cpp.so
```

The complete list is `stage/<arch>/.manifest/aws-crt-cpp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa aws-crt-cpp
ls dists/8.0/*/ | grep '^aws-crt-cpp-'
```

This writes `aws-crt-cpp-<ver>-r0.tar.gz` (run time) and `aws-crt-cpp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add aws-crt-cpp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/aws-crt-cpp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/aws-crt-cpp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_aws_crt_cpp`](../../tests/src/) links aws-crt-cpp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-aws-crt-cpp`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-aws-crt-cpp
/usr/local/bin/qnx-port-tests/test_aws_crt_cpp
```

```
== aws-crt-cpp ==
  ok   ...
PASS aws-crt-cpp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_aws_crt_cpp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_aws_crt_cpp`](../../tests/src/) -- a program linked against aws-crt-cpp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - SHA-256(\"abc\")
   - MD5(\"\")
   - HMAC-SHA256 (RFC 4231 #2)
   - streaming SHA-1 object
   - Base64Encode
   - Base64Decode round trip
   - JSON written then parsed
   - JSON members read back
   - JSON array read back
   - malformed JSON rejected
   - UUID parse/format
   - random UUIDs differ
   - URI scheme, host and port
   - URI path and query
   - DateTime parses RFC 822
   - EventLoopGroup with one thread
   - ResolveHost started
   - loopback literal resolves to itself
   - resolver answered within 10 s

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aws-crt-cpp 0.38.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/aws-crt-cpp-0.38.5-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aws-crt-cpp 0.38.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/aws-crt-cpp-0.38.5-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aws-crt-cpp 0.38.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/aws-crt-cpp-0.38.5-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aws-crt-cpp 0.38.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/aws-crt-cpp-0.38.5-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.38.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add aws-crt-cpp=<version>`), sources in `ported/aws-crt-cpp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates aws-crt-cpp`; port one with `scripts/port-update aws-crt-cpp`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.38.5; GIT=https://github.com/awslabs/aws-crt-cpp.git; REF=v0.38.5
DEPS="aws-c-auth aws-c-mqtt aws-c-event-stream aws-c-s3"
# BUILD_DEPS=OFF: build against the separately packaged aws-c-* components
# instead of the copies in crt/ (whose submodules are therefore not needed).
do_fetch() { git -c advice.detachedHead=false clone -q --depth 1 -b "$REF" "$GIT" "$SRCROOT"; }
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_DEPS=OFF -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" -DAWS_WARNINGS_ARE_ERRORS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
