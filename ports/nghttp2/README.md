# nghttp2 1.64.0

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/nghttp2/nghttp2.git (tag `v1.64.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) [`openssl`](../openssl/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_nghttp2`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add nghttp2
apk add nghttp2-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-nghttp2 && /usr/local/bin/qnx-port-tests/test_nghttp2    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install nghttp2`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild nghttp2` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild zlib openssl
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.64.0 --recurse-submodules --shallow-submodules \
    https://github.com/nghttp2/nghttp2.git src/nghttp2
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/nghttp2 -B build/$QNX_ARCH/nghttp2 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DENABLE_LIB_ONLY=ON \
      -DENABLE_DOC=OFF \
      -DBUILD_TESTING=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/nghttp2 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/nghttp2
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH nghttp2`.

### 5. What it installs

10 files under `/usr/local`: 3 shared-library entries, 2 headers.

```
/usr/local/lib/libnghttp2.so
/usr/local/lib/libnghttp2.so.14
/usr/local/lib/libnghttp2.so.14.28.3
```

The complete list is `stage/<arch>/.manifest/nghttp2` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa nghttp2
ls dists/8.0/*/ | grep '^nghttp2-'
```

This writes `nghttp2-<ver>-r0.tar.gz` (run time) and `nghttp2-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add nghttp2
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/nghttp2-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/nghttp2-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_nghttp2`](../../tests/src/) links nghttp2 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-nghttp2`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-nghttp2
/usr/local/bin/qnx-port-tests/test_nghttp2
```

```
== nghttp2 ==
  ok   ...
PASS nghttp2
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_nghttp2` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_nghttp2`](../../tests/src/) -- a program linked against nghttp2 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - nghttp2_version
   - client session
   - server session
   - nghttp2_submit_request
   - client flush
   - server received the HEADERS frame
   - submit_response
   - server flush
   - client received the response HEADERS

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | nghttp2 1.64.0-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.64.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add nghttp2=<version>`), sources in `ported/nghttp2/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates nghttp2`; port one with `scripts/port-update nghttp2`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.64.0; GIT=https://github.com/nghttp2/nghttp2.git; REF=v1.64.0
DEPS="zlib openssl"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DENABLE_LIB_ONLY=ON -DENABLE_DOC=OFF
                   -DBUILD_TESTING=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
