# paho.mqtt.c 1.3.14

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/eclipse-paho/paho.mqtt.c.git (tag `v1.3.14`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | [`openssl`](../openssl/) |
| Run-time dependencies | [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install paho.mqtt.c
oss-get install paho.mqtt.c-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild paho.mqtt.c` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild openssl
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.3.14 --recurse-submodules --shallow-submodules \
    https://github.com/eclipse-paho/paho.mqtt.c.git src/paho.mqtt.c
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/paho.mqtt.c < ports/paho.mqtt.c/patches/0001-qnx-bool-vs-stdbool.patch
patch -p1 -d src/paho.mqtt.c < ports/paho.mqtt.c/patches/0002-qnx-endian-netbyte.patch
```

**[`0001-qnx-bool-vs-stdbool.patch`](patches/0001-qnx-bool-vs-stdbool.patch)** — Paho MQTT C: do not redefine bool when <stdbool.h> is in scope.

> With PAHO_WITH_SSL the OpenSSL headers bring in <stdbool.h>, which makes bool
> a macro for _Bool; the typedef then reads "typedef unsigned int _Bool" and is
> rejected.

Touches: `src/MQTTPacket.h` · Upstream status: Pending

**[`0002-qnx-endian-netbyte.patch`](patches/0002-qnx-endian-netbyte.patch)** — Paho MQTT C: byte-order helpers from <net/netbyte.h> on QNX.

> QNX has no <endian.h>; htobeNN/beNNtoh are declared in <net/netbyte.h>.

Touches: `src/SHA1.c`, `src/WebSocket.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/paho.mqtt.c -B build/$QNX_ARCH/paho.mqtt.c -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DPAHO_BUILD_SHARED=ON \
      -DPAHO_BUILD_STATIC=ON \
      -DPAHO_WITH_SSL=ON \
      -DPAHO_BUILD_SAMPLES=OFF \
      -DPAHO_BUILD_DOCUMENTATION=OFF \
      -DPAHO_ENABLE_TESTING=OFF \
      -DOPENSSL_ROOT_DIR="$STAGE$PREFIX"
cmake --build build/$QNX_ARCH/paho.mqtt.c -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/paho.mqtt.c
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH paho.mqtt.c`.

### 5. What it installs

43 files under `/usr/local`: 12 shared-library entries, 1 programs, 7 headers.

```
/usr/local/lib/libpaho-mqtt3a.so
/usr/local/lib/libpaho-mqtt3a.so.1
/usr/local/lib/libpaho-mqtt3a.so.1.3.14
/usr/local/lib/libpaho-mqtt3as.so
/usr/local/lib/libpaho-mqtt3as.so.1
/usr/local/lib/libpaho-mqtt3as.so.1.3.14
/usr/local/lib/libpaho-mqtt3c.so
/usr/local/lib/libpaho-mqtt3c.so.1
/usr/local/lib/libpaho-mqtt3c.so.1.3.14
/usr/local/lib/libpaho-mqtt3cs.so
/usr/local/lib/libpaho-mqtt3cs.so.1
/usr/local/lib/libpaho-mqtt3cs.so.1.3.14
/usr/local/bin/MQTTVersion
```

The complete list is `stage/<arch>/.manifest/paho.mqtt.c` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH paho.mqtt.c
ls repo/8.0/*/ | grep '^paho.mqtt.c-'
```

This writes `paho.mqtt.c-<ver>-r0.tar.gz` (run time) and `paho.mqtt.c-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install paho.mqtt.c
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/paho.mqtt.c-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/paho.mqtt.c-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.3.14; GIT=https://github.com/eclipse-paho/paho.mqtt.c.git; REF=v1.3.14
DEPS="openssl"
CMAKE_ARGS_EXTRA=( -DPAHO_BUILD_SHARED=ON -DPAHO_BUILD_STATIC=ON
                   -DPAHO_WITH_SSL=ON -DPAHO_BUILD_SAMPLES=OFF
                   -DPAHO_BUILD_DOCUMENTATION=OFF -DPAHO_ENABLE_TESTING=OFF
                   -DOPENSSL_ROOT_DIR="$STAGE$PREFIX" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
