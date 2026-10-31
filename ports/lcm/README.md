# lcm 1.5.2

> Lightweight Communications and Marshalling: message passing for real time

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/lcm-proj/lcm.git (tag `v1.5.2`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`glib`](../glib/) |
| Run-time dependencies | [`glib`](../glib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_lcm`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add lcm
apk add lcm-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-lcm && /usr/local/bin/qnx-port-tests/test_lcm    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install lcm`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild lcm` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (9 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib
```

<details><summary>Full build chain (9)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.5.2 --recurse-submodules --shallow-submodules \
    https://github.com/lcm-proj/lcm.git src/lcm
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/lcm < ports/lcm/patches/0001-include-sys-socket.patch
```

**[`0001-include-sys-socket.patch`](patches/0001-include-sys-socket.patch)** — Include <sys/socket.h> for struct sockaddr

> lcm_buf_t has a struct sockaddr member, and the header includes
> <arpa/inet.h> and <netdb.h> but not <sys/socket.h>, where struct sockaddr is
> declared. glibc's headers pull it in through those; QNX's do not ("field
> 'from' has incomplete type").

Touches: `lcm/udpm_util.h` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/lcm -B build/$QNX_ARCH/lcm -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DLCM_ENABLE_PYTHON=OFF \
      -DLCM_ENABLE_JAVA=OFF \
      -DLCM_ENABLE_LUA=OFF \
      -DLCM_ENABLE_GO=OFF \
      -DLCM_ENABLE_TESTS=OFF \
      -DLCM_ENABLE_EXAMPLES=OFF \
      -DLCM_INSTALL_M4MACROS=OFF \
      -DLCM_ENABLE_DOCS=OFF
cmake --build build/$QNX_ARCH/lcm -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/lcm
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- lcm: Lightweight Communications and Marshalling -- message types (lcm-gen) and UDP multicast publish/subscribe.
- The C and C++ libraries and lcm-gen; no Python/Java/Lua/Go bindings.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH lcm`.

### 5. What it installs

24 files under `/usr/local`: 3 shared-library entries, 3 programs, 8 headers.

```
/usr/local/lib/liblcm.so
/usr/local/lib/liblcm.so.1
/usr/local/lib/liblcm.so.1.5.2
/usr/local/bin/lcm-gen
/usr/local/bin/lcm-logger
/usr/local/bin/lcm-logplayer
```

The complete list is `stage/<arch>/.manifest/lcm` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa lcm
ls dists/8.0/*/ | grep '^lcm-'
```

This writes `lcm-<ver>-r0.tar.gz` (run time) and `lcm-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add lcm
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/lcm-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/lcm-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_lcm`](../../tests/src/) links lcm the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-lcm`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-lcm
/usr/local/bin/qnx-port-tests/test_lcm
```

```
== lcm ==
  ok   ...
PASS lcm
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_lcm` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_lcm`](../../tests/src/) -- a program linked against lcm the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - lcm_create(memq://)
   - lcm_subscribe (regex channel)
   - lcm_publish
   - publish on another channel
   - lcm_handle_timeout
   - exactly one message received
   - channel and payload

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lcm 1.5.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/lcm-1.5.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lcm 1.5.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/lcm-1.5.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.5.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add lcm=<version>`), sources in `ported/lcm/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates lcm`; port one with `scripts/port-update lcm`.

<details><summary>recipe.sh</summary>

```bash
# lcm: Lightweight Communications and Marshalling -- message types (lcm-gen) and UDP multicast publish/subscribe.
VERSION=1.5.2; GIT=https://github.com/lcm-proj/lcm.git; REF=v1.5.2
DEPS="glib"
BUILDSYS=cmake
# The C and C++ libraries and lcm-gen; no Python/Java/Lua/Go bindings.
CMAKE_ARGS_EXTRA=( -DLCM_ENABLE_PYTHON=OFF -DLCM_ENABLE_JAVA=OFF -DLCM_ENABLE_LUA=OFF -DLCM_ENABLE_GO=OFF -DLCM_ENABLE_TESTS=OFF -DLCM_ENABLE_EXAMPLES=OFF -DLCM_INSTALL_M4MACROS=OFF -DLCM_ENABLE_DOCS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
