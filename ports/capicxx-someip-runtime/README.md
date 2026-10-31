# capicxx-someip-runtime 3.2.4

> COVESA CommonAPI C++ SOME/IP binding runtime

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/COVESA/capicxx-someip-runtime.git (tag `3.2.4`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | [`capicxx-core-runtime`](../capicxx-core-runtime/) [`vsomeip`](../vsomeip/) |
| Run-time dependencies | [`capicxx-core-runtime`](../capicxx-core-runtime/) [`vsomeip`](../vsomeip/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_capicxx_someip_runtime`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add capicxx-someip-runtime
apk add capicxx-someip-runtime-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-capicxx-someip-runtime && /usr/local/bin/qnx-port-tests/test_capicxx_someip_runtime    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install capicxx-someip-runtime`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild capicxx-someip-runtime` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (10 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild capicxx-core-runtime vsomeip
```

<details><summary>Full build chain (10)</summary>

[`boost`](../boost/) [`bzip2`](../bzip2/) [`capicxx-core-runtime`](../capicxx-core-runtime/) [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`vsomeip`](../vsomeip/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.2.4 --recurse-submodules --shallow-submodules \
    https://github.com/COVESA/capicxx-someip-runtime.git src/capicxx-someip-runtime
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/capicxx-someip-runtime < ports/capicxx-someip-runtime/patches/0001-qnx-bsd-endian-macros.patch
patch -p1 -d src/capicxx-someip-runtime < ports/capicxx-someip-runtime/patches/0002-qnx-signed-uid-narrowing.patch
```

**[`0001-qnx-bsd-endian-macros.patch`](patches/0001-qnx-bsd-endian-macros.patch)** — CommonAPI SOME/IP: byte-order macros on QNX.

> The streams test glibc's __BYTE_ORDER/__LITTLE_ENDIAN; QNX has only the BSD
> names from <sys/param.h>. Alias them on QNX.

Touches: `include/CommonAPI/SomeIP/OutputStream.hpp`, `include/CommonAPI/SomeIP/InputStream.hpp`, `src/CommonAPI/SomeIP/StringEncoder.cpp` · Upstream status: QNX-specific

**[`0002-qnx-signed-uid-narrowing.patch`](patches/0002-qnx-signed-uid-narrowing.patch)** — CommonAPI SOME/IP: write "no uid/gid" as (uid_t)-1.

> uid_t and gid_t are signed on QNX, so the brace-initialisers 0xffffffff are
> narrowing conversions, which C++ rejects. (uid_t)-1 is the same value where
> the type is unsigned.

Touches: `src/CommonAPI/SomeIP/ClientId.cpp` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/capicxx-someip-runtime -B build/$QNX_ARCH/capicxx-someip-runtime -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DUSE_INSTALLED_COMMONAPI=ON \
      -DCommonAPI_DIR="$STAGE$PREFIX/lib/cmake/CommonAPI" \
      -Dvsomeip3_DIR="$STAGE$PREFIX/lib/cmake/vsomeip3"
cmake --build build/$QNX_ARCH/capicxx-someip-runtime -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/capicxx-someip-runtime
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH capicxx-someip-runtime`.

### 5. What it installs

41 files under `/usr/local`: 2 shared-library entries, 34 headers.

```
/usr/local/lib/libCommonAPI-SomeIP.so
/usr/local/lib/libCommonAPI-SomeIP.so.3.2.4
```

The complete list is `stage/<arch>/.manifest/capicxx-someip-runtime` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa capicxx-someip-runtime
ls dists/8.0/*/ | grep '^capicxx-someip-runtime-'
```

This writes `capicxx-someip-runtime-<ver>-r0.tar.gz` (run time) and `capicxx-someip-runtime-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add capicxx-someip-runtime
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/capicxx-someip-runtime-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/capicxx-someip-runtime-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_capicxx_someip_runtime`](../../tests/src/) links capicxx-someip-runtime the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-capicxx-someip-runtime`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-capicxx-someip-runtime
/usr/local/bin/qnx-port-tests/test_capicxx_someip_runtime
```

```
== capicxx-someip-runtime ==
  ok   ...
PASS capicxx-someip-runtime
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_capicxx_someip_runtime` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_capicxx_someip_runtime`](../../tests/src/) -- a program linked against capicxx-someip-runtime the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - AddressTranslator singleton
   - CommonAPI address -> SOME/IP service 0x1234 instance 0x5678
   - SOME/IP address -> CommonAPI address
   - method call message for service 0x1234 method 0x0421
   - serialise uint32, int32, bool, double, string
   - big-endian wire bytes for the fixed-size values
   - string bytes present after its length field
   - deserialise without error
   - values read back unchanged
   - response message mirrors method and session

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | capicxx-someip-runtime 3.2.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/capicxx-someip-runtime-3.2.4-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | capicxx-someip-runtime 3.2.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/capicxx-someip-runtime-3.2.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | capicxx-someip-runtime 3.2.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/capicxx-someip-runtime-3.2.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | capicxx-someip-runtime 3.2.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/capicxx-someip-runtime-3.2.4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.2.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add capicxx-someip-runtime=<version>`), sources in `ported/capicxx-someip-runtime/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates capicxx-someip-runtime`; port one with `scripts/port-update capicxx-someip-runtime`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.2.4; GIT=https://github.com/COVESA/capicxx-someip-runtime.git; REF=3.2.4
DEPS="capicxx-core-runtime vsomeip"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                   -DUSE_INSTALLED_COMMONAPI=ON
                   -DCommonAPI_DIR="$STAGE$PREFIX/lib/cmake/CommonAPI"
                   -Dvsomeip3_DIR="$STAGE$PREFIX/lib/cmake/vsomeip3" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
