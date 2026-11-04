# libvalkey 0.5.0

> Valkey client library in C (with cluster and TLS support)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/valkey-io/libvalkey.git (tag `0.5.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`openssl`](../openssl/) |
| Run-time dependencies | [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libvalkey`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libvalkey` (programs, shared libraries, run-time data) and `libvalkey-dev` (headers, pkg-config/CMake files, static libraries; needs `libvalkey`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libvalkey`](https://openqnx.github.io/oss/package/x86_64/libvalkey.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libvalkey.html)) · [`libvalkey-dev`](https://openqnx.github.io/oss/package/x86_64/libvalkey-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libvalkey-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libvalkey` (dependencies come along); one version: `apk add libvalkey=<version>` | `apk del libvalkey` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libvalkey` (dependencies first) | `oss-get remove libvalkey` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libvalkey-dev` the same way, by its own name (`apk add libvalkey-dev`, `oss-get install libvalkey-dev`, by hand with its own file and list `libvalkey-dev.list`); remove it before `libvalkey`, which it needs.

```sh
# on the QNX target, as root
apk add libvalkey libvalkey-dev
apk add qnx-test-libvalkey && /usr/local/bin/qnx-port-tests/test_libvalkey    # its on-target test, from tests-8.0-<arch>
apk del libvalkey libvalkey-dev qnx-test-libvalkey
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libvalkey.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libvalkey.list
rm var/lib/manual/libvalkey.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libvalkey` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 0.5.0 --recurse-submodules --shallow-submodules \
    https://github.com/valkey-io/libvalkey.git src/libvalkey
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libvalkey -B build/$QNX_ARCH/libvalkey -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DENABLE_TLS=ON \
      -DENABLE_RDMA=OFF \
      -DDISABLE_TESTS=ON \
      -DBUILD_SHARED_LIBS=ON
cmake --build build/$QNX_ARCH/libvalkey -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libvalkey
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libvalkey: the Valkey project's C client library (a hiredis fork with cluster support), with its TLS layer (libvalkey_tls) against the staged OpenSSL. RDMA support is Linux-only (rdma-core) and stays off.
- Tests off: they need a running valkey-server on the build machine.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libvalkey`.

### 5. What it installs

35 files under `/usr/local`: 6 shared-library entries, 20 headers.

```
/usr/local/lib/libvalkey.so
/usr/local/lib/libvalkey.so.0
/usr/local/lib/libvalkey.so.0.5.0
/usr/local/lib/libvalkey_tls.so
/usr/local/lib/libvalkey_tls.so.0
/usr/local/lib/libvalkey_tls.so.0.5.0
```

The complete list is `stage/<arch>/.manifest/libvalkey` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libvalkey
ls dists/8.0/*/ | grep '^libvalkey-'
```

This writes `libvalkey-<ver>-r0.tar.gz` (run time) and `libvalkey-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libvalkey
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libvalkey-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libvalkey-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libvalkey`](../../tests/src/) links libvalkey the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libvalkey`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libvalkey
/usr/local/bin/qnx-port-tests/test_libvalkey
```

```
== libvalkey ==
  ok   ...
PASS libvalkey
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libvalkey` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libvalkey`](../../tests/src/) -- a program linked against libvalkey the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - reader: an incomplete reply yields nothing yet
   - reader: array [\"foo\", 42, nil]
   - valkeyFormatCommand: exact RESP bytes (binary %b)
   - self-signed certificate for the TLS port
   - valkeyConnect to valkey-server on 127.0.0.1, PING -> PONG
   - SET of a binary value
   - GET returns the 6 binary bytes
   - INCR: 42
   - GET of a missing key: nil
   - INCR of a non-integer: error reply
   - pipelining: three commands, replies in order
   - RESP3: HELLO 3, HGETALL is a map reply
   - valkeyConnectUnix: GET over the UNIX socket
   - async API (poll adapter): SET and GET callbacks, then disconnect
   - valkeyCreateTLSContext trusting the self-signed certificate
   - TLS handshake with valkey-server's tls-port
   - SET/GET over TLS
   - CLUSTER ADDSLOTSRANGE 0 16383
   - CLUSTER KEYSLOT foo = 12182
   - cluster-mode valkey-server reaches cluster_state:ok
   - valkeyClusterConnect fetched the slot map
   - the cluster has one node
   - valkeyClusterCommand: SET/GET of 20 keys routed by slot

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> libvalkey`.

## Versions

This recipe builds **0.5.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libvalkey=<version>`), sources in `ported/libvalkey/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libvalkey`; port one with `scripts/port-update libvalkey`.

<details><summary>recipe.sh</summary>

```bash
# libvalkey: the Valkey project's C client library (a hiredis fork with
# cluster support), with its TLS layer (libvalkey_tls) against the staged
# OpenSSL. RDMA support is Linux-only (rdma-core) and stays off.
VERSION=0.5.0; GIT=https://github.com/valkey-io/libvalkey.git; REF=0.5.0
DEPS="openssl"
BUILDSYS=cmake
# Tests off: they need a running valkey-server on the build machine.
CMAKE_ARGS_EXTRA=( -DENABLE_TLS=ON -DENABLE_RDMA=OFF -DDISABLE_TESTS=ON -DBUILD_SHARED_LIBS=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
