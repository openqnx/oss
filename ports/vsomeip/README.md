# vsomeip 3.5.6

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/COVESA/vsomeip.git (tag `3.5.6`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`boost`](../boost/) |
| Run-time dependencies | [`boost`](../boost/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_vsomeip`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add vsomeip
apk add vsomeip-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-vsomeip && /usr/local/bin/qnx-port-tests/test_vsomeip    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install vsomeip`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild vsomeip` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild boost
```

<details><summary>Full build chain (8)</summary>

[`boost`](../boost/) [`bzip2`](../bzip2/) [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.5.6 --recurse-submodules --shallow-submodules \
    https://github.com/COVESA/vsomeip.git src/vsomeip
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/vsomeip < ports/vsomeip/patches/0001-qnx8-native-accept4.patch
```

**[`0001-qnx8-native-accept4.patch`](patches/0001-qnx8-native-accept4.patch)** — vsomeip: use QNX 8.0's own accept4().

> The QNX wrapper re-implements accept4() with the _IO_OPENFD message protocol
> for QNX 7.x. QNX 8.0 exports accept4() from libsocket and no longer has
> <sys/sockmsg.h>, which the emulation needs; build it for pre-8.0 targets only.

Touches: `implementation/utility/src/wrappers_qnx.cpp` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/vsomeip -B build/$QNX_ARCH/vsomeip -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DENABLE_SIGNAL_HANDLING=1 \
      -DBoost_NO_BOOST_CMAKE=OFF \
      -DBOOST_ROOT="$STAGE$PREFIX" \
      -DBoost_INCLUDE_DIR="$STAGE$PREFIX/include" \
      -DBoost_LIBRARY_DIR="$STAGE$PREFIX/lib" \
      -DDISABLE_DLT=1
cmake --build build/$QNX_ARCH/vsomeip -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/vsomeip
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH vsomeip`.

### 5. What it installs

69 files under `/usr/local`: 12 shared-library entries, 46 headers.

```
/usr/local/lib/libvsomeip3-cfg.so
/usr/local/lib/libvsomeip3-cfg.so.3
/usr/local/lib/libvsomeip3-cfg.so.3.5.6
/usr/local/lib/libvsomeip3-e2e.so
/usr/local/lib/libvsomeip3-e2e.so.3
/usr/local/lib/libvsomeip3-e2e.so.3.5.6
/usr/local/lib/libvsomeip3-sd.so
/usr/local/lib/libvsomeip3-sd.so.3
/usr/local/lib/libvsomeip3-sd.so.3.5.6
/usr/local/lib/libvsomeip3.so
/usr/local/lib/libvsomeip3.so.3
/usr/local/lib/libvsomeip3.so.3.5.6
```

The complete list is `stage/<arch>/.manifest/vsomeip` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa vsomeip
ls dists/8.0/*/ | grep '^vsomeip-'
```

This writes `vsomeip-<ver>-r0.tar.gz` (run time) and `vsomeip-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add vsomeip
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/vsomeip-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/vsomeip-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_vsomeip`](../../tests/src/) links vsomeip the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-vsomeip`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-vsomeip
/usr/local/bin/qnx-port-tests/test_vsomeip
```

```
== vsomeip ==
  ok   ...
PASS vsomeip
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_vsomeip` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_vsomeip`](../../tests/src/) -- a program linked against vsomeip the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - runtime::get
   - create_application
   - application name readback
   - create_payload
   - payload length
   - payload content
   - create_request
   - service id readback
   - method id readback
   - payload attached to message

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | vsomeip 3.5.6-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | vsomeip 3.5.6-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **3.5.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add vsomeip=<version>`), sources in `ported/vsomeip/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates vsomeip`; port one with `scripts/port-update vsomeip`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.5.6; GIT=https://github.com/COVESA/vsomeip.git; REF=3.5.6
DEPS="boost"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DENABLE_SIGNAL_HANDLING=1
                   -DBoost_NO_BOOST_CMAKE=OFF
                   -DBOOST_ROOT="$STAGE$PREFIX"
                   -DBoost_INCLUDE_DIR="$STAGE$PREFIX/include"
                   -DBoost_LIBRARY_DIR="$STAGE$PREFIX/lib"
                   -DDISABLE_DLT=1 )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
