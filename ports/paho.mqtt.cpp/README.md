# paho.mqtt.cpp 1.5.2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/eclipse-paho/paho.mqtt.cpp.git (tag `v1.5.2`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`paho.mqtt.c`](../paho.mqtt.c/) |
| Run-time dependencies | [`openssl`](../openssl/) [`paho.mqtt.c`](../paho.mqtt.c/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add paho.mqtt.cpp
apk add paho.mqtt.cpp-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install paho.mqtt.cpp`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild paho.mqtt.cpp` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (2 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild paho.mqtt.c
```

<details><summary>Full build chain (2)</summary>

[`openssl`](../openssl/) [`paho.mqtt.c`](../paho.mqtt.c/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.5.2 --recurse-submodules --shallow-submodules \
    https://github.com/eclipse-paho/paho.mqtt.cpp.git src/paho.mqtt.cpp
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/paho.mqtt.cpp -B build/$QNX_ARCH/paho.mqtt.cpp -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DPAHO_BUILD_SHARED=ON \
      -DPAHO_BUILD_STATIC=ON \
      -DPAHO_WITH_MQTT_C=OFF \
      -DPAHO_WITH_SSL=ON \
      -DPAHO_BUILD_SAMPLES=OFF \
      -DPAHO_BUILD_TESTS=OFF \
      -DPAHO_BUILD_DOCUMENTATION=OFF \
      -DOPENSSL_ROOT_DIR="$STAGE$PREFIX"
cmake --build build/$QNX_ARCH/paho.mqtt.cpp -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/paho.mqtt.cpp
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH paho.mqtt.cpp`.

### 5. What it installs

38 files under `/usr/local`: 3 shared-library entries, 30 headers.

```
/usr/local/lib/libpaho-mqttpp3.so
/usr/local/lib/libpaho-mqttpp3.so.1
/usr/local/lib/libpaho-mqttpp3.so.1.5.1
```

The complete list is `stage/<arch>/.manifest/paho.mqtt.cpp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa paho.mqtt.cpp
ls dists/8.0/*/ | grep '^paho.mqtt.cpp-'
```

This writes `paho.mqtt.cpp-<ver>-r0.tar.gz` (run time) and `paho.mqtt.cpp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add paho.mqtt.cpp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/paho.mqtt.cpp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/paho.mqtt.cpp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

## Versions

This recipe builds **1.5.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add paho.mqtt.cpp=<version>`), sources in `ported/paho.mqtt.cpp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates paho.mqtt.cpp`; port one with `scripts/port-update paho.mqtt.cpp`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.5.2; GIT=https://github.com/eclipse-paho/paho.mqtt.cpp.git; REF=v1.5.2
DEPS="paho.mqtt.c"
CMAKE_ARGS_EXTRA=( -DPAHO_BUILD_SHARED=ON -DPAHO_BUILD_STATIC=ON
                   -DPAHO_WITH_MQTT_C=OFF -DPAHO_WITH_SSL=ON
                   -DPAHO_BUILD_SAMPLES=OFF -DPAHO_BUILD_TESTS=OFF
                   -DPAHO_BUILD_DOCUMENTATION=OFF
                   -DOPENSSL_ROOT_DIR="$STAGE$PREFIX" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
