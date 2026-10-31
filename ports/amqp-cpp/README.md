# amqp-cpp 4.3.27

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git (tag `v4.3.27`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`openssl`](../openssl/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add amqp-cpp
apk add amqp-cpp-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install amqp-cpp`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild amqp-cpp` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v4.3.27 --recurse-submodules --shallow-submodules \
    https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git src/amqp-cpp
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/amqp-cpp < ports/amqp-cpp/patches/0001-addressinfo-include-netinet-in.patch
```

**[`0001-addressinfo-include-netinet-in.patch`](patches/0001-addressinfo-include-netinet-in.patch)** — Include <netinet/in.h> for sockaddr_in/sockaddr_in6.

> addressinfo.h compares IPv4 and IPv6 addresses through struct sockaddr_in
> and sockaddr_in6 but only includes <netdb.h>. glibc's <netdb.h> happens to
> pull in <netinet/in.h>; QNX's does not, so the structs are incomplete there.

Touches: `src/linux_tcp/addressinfo.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/amqp-cpp -B build/$QNX_ARCH/amqp-cpp -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DAMQP-CPP_BUILD_SHARED=ON \
      -DAMQP-CPP_LINUX_TCP=ON
cmake --build build/$QNX_ARCH/amqp-cpp -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/amqp-cpp
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH amqp-cpp`.

### 5. What it installs

68 files under `/usr/local`: 2 shared-library entries, 63 headers.

```
/usr/local/lib/libamqpcpp.so
/usr/local/lib/libamqpcpp.so.4.3
```

The complete list is `stage/<arch>/.manifest/amqp-cpp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa amqp-cpp
ls dists/8.0/*/ | grep '^amqp-cpp-'
```

This writes `amqp-cpp-<ver>-r0.tar.gz` (run time) and `amqp-cpp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add amqp-cpp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/amqp-cpp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/amqp-cpp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | amqp-cpp 4.3.27-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/amqp-cpp-4.3.27-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | amqp-cpp 4.3.27-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/amqp-cpp-4.3.27-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.3.27**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add amqp-cpp=<version>`), sources in `ported/amqp-cpp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates amqp-cpp`; port one with `scripts/port-update amqp-cpp`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.3.27; GIT=https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git; REF=v4.3.27
DEPS="openssl"
CMAKE_ARGS_EXTRA=( -DAMQP-CPP_BUILD_SHARED=ON -DAMQP-CPP_LINUX_TCP=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
