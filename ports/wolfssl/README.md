# wolfssl 5.7.6

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/wolfSSL/wolfssl.git (tag `v5.7.6-stable`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_wolfssl`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install wolfssl
oss-get install wolfssl-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild wolfssl` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
git clone --depth 1 -b v5.7.6-stable --recurse-submodules --shallow-submodules \
    https://github.com/wolfSSL/wolfssl.git src/wolfssl
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/wolfssl -B build/$QNX_ARCH/wolfssl -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCMAKE_C_FLAGS="$CFLAGS_EXTRA" \
      -DBUILD_SHARED_LIBS=ON \
      -DWOLFSSL_EXAMPLES=OFF \
      -DWOLFSSL_CRYPT_TESTS=OFF \
      -DWOLFSSL_TLS13=ON \
      -DWOLFSSL_DTLS=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/wolfssl -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/wolfssl
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GCC 12 raises a false-positive -Wstringop-overflow in tls.c; wolfSSL turns warnings into errors by default, so demote just that one.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH wolfssl`.

### 5. What it installs

187 files under `/usr/local`: 3 shared-library entries, 168 headers.

```
/usr/local/lib/libwolfssl.so
/usr/local/lib/libwolfssl.so.43
/usr/local/lib/libwolfssl.so.43.0.0
```

The complete list is `stage/<arch>/.manifest/wolfssl` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH wolfssl
ls repo/8.0/*/ | grep '^wolfssl-'
```

This writes `wolfssl-<ver>-r0.tar.gz` (run time) and `wolfssl-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install wolfssl
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/wolfssl-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/wolfssl-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_wolfssl`](../../tests/src/) links wolfssl the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_wolfssl root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_wolfssl'
```

```
== wolfssl ==
  ok   ...
PASS wolfssl
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=5.7.6; GIT=https://github.com/wolfSSL/wolfssl.git; REF=v5.7.6-stable
# GCC 12 raises a false-positive -Wstringop-overflow in tls.c; wolfSSL turns
# warnings into errors by default, so demote just that one.
CFLAGS_EXTRA="-Wno-error=stringop-overflow"
CMAKE_ARGS_EXTRA=( -DCMAKE_C_FLAGS="$CFLAGS_EXTRA" -DBUILD_SHARED_LIBS=ON -DWOLFSSL_EXAMPLES=OFF
                   -DWOLFSSL_CRYPT_TESTS=OFF -DWOLFSSL_TLS13=ON
                   -DWOLFSSL_DTLS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
