# azure-iot-sdk-c LTS_08_2024

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/Azure/azure-iot-sdk-c.git (tag `LTS_08_2024`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | [`openssl`](../openssl/) [`curl`](../curl/) |
| Run-time dependencies | [`curl`](../curl/) [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add azure-iot-sdk-c
apk add azure-iot-sdk-c-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install azure-iot-sdk-c`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild azure-iot-sdk-c` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild openssl curl
```

<details><summary>Full build chain (7)</summary>

[`brotli`](../brotli/) [`c-ares`](../c-ares/) [`curl`](../curl/) [`nghttp2`](../nghttp2/) [`openssl`](../openssl/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b LTS_08_2024 --recurse-submodules --shallow-submodules \
    https://github.com/Azure/azure-iot-sdk-c.git src/azure-iot-sdk-c
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/azure-iot-sdk-c < ports/azure-iot-sdk-c/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/azure-iot-sdk-c < ports/azure-iot-sdk-c/patches/0002-c-utility-qnx-from-qnx-ports-build-files.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add QNX support

> Imported from qnx-ports/azure-iot-sdk-c (LTS_08_2024...qnx-LTS_08_2024); original authorship kept.
>
>
> ---
>  CMakeLists.txt                                 | 2 ++
>  serializer/tests/serializer_e2e/CMakeLists.txt | 5 ++++-
>  testtools/iothub_test/src/iothub_account.c     | 4 ++--
>  3 files changed, 8 insertions(+), 3 deletions(-)

Touches: `CMakeLists.txt`, `serializer/tests/serializer_e2e/CMakeLists.txt`, `testtools/iothub_test/src/iothub_account.c` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-c-utility-qnx-from-qnx-ports-build-files.patch`](patches/0002-c-utility-qnx-from-qnx-ports-build-files.patch)** — QNX support in the c-utility submodule (azure-c-shared-utility).

> paths rebased onto the top-level tree (the submodule lives in c-utility/).

Touches: `c-utility/CMakeLists.txt`, `c-utility/adapters/socketio_berkeley.c`, `c-utility/configs/azure_c_shared_utilityFunctions.cmake`, `c-utility/configs/azure_iot_build_rules.cmake`, `c-utility/testtools/micromock/CMakeLists.txt` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/azure-iot-sdk-c -B build/$QNX_ARCH/azure-iot-sdk-c -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -Duse_openssl=ON \
      -Duse_mqtt=ON \
      -Duse_amqp=ON \
      -Duse_http=ON \
      -Dskip_samples=ON \
      -Drun_unittests=OFF \
      -Drun_e2e_tests=OFF \
      -Duse_prov_client=OFF \
      -Dbuild_service_client=OFF \
      -Dbuild_provisioning_service_client=OFF \
      -Ddont_use_uploadtoblob=ON \
      -Duse_installed_dependencies=OFF \
      -Dbuild_as_dynamic=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_C_STANDARD=99 \
      -Duse_default_uuid=ON
cmake --build build/$QNX_ARCH/azure-iot-sdk-c -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/azure-iot-sdk-c
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH azure-iot-sdk-c`.

### 5. What it installs

286 files under `/usr/local`: 7 shared-library entries, 238 headers.

```
/usr/local/lib/libaziotsharedutil_dll.so
/usr/local/lib/libiothub_client.so
/usr/local/lib/libiothub_client.so.1
/usr/local/lib/libiothub_client.so.1.13.0
/usr/local/lib/libserializer.so
/usr/local/lib/libserializer.so.1
/usr/local/lib/libserializer.so.1.13.0
```

The complete list is `stage/<arch>/.manifest/azure-iot-sdk-c` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa azure-iot-sdk-c
ls dists/8.0/*/ | grep '^azure-iot-sdk-c-'
```

This writes `azure-iot-sdk-c-<ver>-r0.tar.gz` (run time) and `azure-iot-sdk-c-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add azure-iot-sdk-c
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/azure-iot-sdk-c-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/azure-iot-sdk-c-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | azure-iot-sdk-c 0_git-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | azure-iot-sdk-c 0_git-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **LTS_08_2024**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add azure-iot-sdk-c=<version>`), sources in `ported/azure-iot-sdk-c/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates azure-iot-sdk-c`; port one with `scripts/port-update azure-iot-sdk-c`.

<details><summary>recipe.sh</summary>

```bash
VERSION=LTS_08_2024; GIT=https://github.com/Azure/azure-iot-sdk-c.git; REF=LTS_08_2024
DEPS="openssl curl"
CMAKE_ARGS_EXTRA=( -Duse_openssl=ON -Duse_mqtt=ON -Duse_amqp=ON -Duse_http=ON -Dskip_samples=ON -Drun_unittests=OFF
                   -Drun_e2e_tests=OFF -Duse_prov_client=OFF -Dbuild_service_client=OFF -Dbuild_provisioning_service_client=OFF
                   -Ddont_use_uploadtoblob=ON -Duse_installed_dependencies=OFF -Dbuild_as_dynamic=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                   -DCMAKE_C_STANDARD=99 -Duse_default_uuid=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
