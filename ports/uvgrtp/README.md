# uvgrtp 4.0.1

> Open-source RTP library written in C++ (Ultra Video Group)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/ultravideo/uvgRTP.git (tag `v4.0.1`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_uvgrtp`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `uvgrtp` (programs, shared libraries, run-time data) and `uvgrtp-dev` (headers, pkg-config/CMake files, static libraries; needs `uvgrtp`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`uvgrtp`](https://openqnx.github.io/oss/package/x86_64/uvgrtp.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/uvgrtp.html)) · [`uvgrtp-dev`](https://openqnx.github.io/oss/package/x86_64/uvgrtp-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/uvgrtp-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add uvgrtp` (dependencies come along); one version: `apk add uvgrtp=<version>` | `apk del uvgrtp` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install uvgrtp` (dependencies first) | `oss-get remove uvgrtp` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`uvgrtp-dev` the same way, by its own name (`apk add uvgrtp-dev`, `oss-get install uvgrtp-dev`, by hand with its own file and list `uvgrtp-dev.list`); remove it before `uvgrtp`, which it needs.

```sh
# on the QNX target, as root
apk add uvgrtp uvgrtp-dev
apk add qnx-test-uvgrtp && /usr/local/bin/qnx-port-tests/test_uvgrtp    # its on-target test, from tests-8.0-<arch>-m-z
apk del uvgrtp uvgrtp-dev qnx-test-uvgrtp
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/uvgrtp.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/uvgrtp.list
rm var/lib/manual/uvgrtp.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild uvgrtp` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v4.0.1 --recurse-submodules --shallow-submodules \
    https://github.com/ultravideo/uvgRTP.git src/uvgrtp
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/uvgrtp < ports/uvgrtp/patches/0001-socket-buffer-size-fallback.patch
```

**[`0001-socket-buffer-size-fallback.patch`](patches/0001-socket-buffer-size-fallback.patch)** — Fall back to a smaller default UDP buffer when the stack refuses 4 MiB.

> Every media stream asks for 4 MiB UDP send and receive buffers and fails
> to initialise when setsockopt() refuses. Linux silently caps the request at
> net.core.wmem_max/rmem_max; QNX's io-sock, like the BSD stacks it derives
> from, refuses a size above kern.ipc.maxsockbuf with ENOBUFS, so no stream
> could be created at all ("Failed to set socket options ... Failed to
> initialize media stream"). Halve the request until the stack accepts it
> (down to 64 KiB), which is what Linux ends up with in effect; a size the
> application sets with RCC_UDP_SND_BUF_SIZE/RCC_UDP_RCV_BUF_SIZE is still
> passed through unchanged.

Touches: `src/media_stream_internal.cc` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/uvgrtp -B build/$QNX_ARCH/uvgrtp -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DUVGRTP_BUILD_SHARED=ON \
      -DUVGRTP_DISABLE_CRYPTO=ON \
      -DUVGRTP_DISABLE_TESTS=ON \
      -DUVGRTP_DISABLE_EXAMPLES=ON \
      -DUVGRTP_RELEASE_COMMIT=ON
cmake --build build/$QNX_ARCH/uvgrtp -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/uvgrtp
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- uvgRTP: RTP library (RFC 3550) in C++ with RTCP, and payload formats for H.264, H.265, H.266 (VVC) and Opus, from Ultra Video Group.
- Shared library. SRTP/ZRTP need Crypto++, which is not ported: off. No unit tests (they download googletest) and no examples.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH uvgrtp`.

### 5. What it installs

21 files under `/usr/local`: 3 shared-library entries, 12 headers.

```
/usr/local/lib/libuvgrtp.so
/usr/local/lib/libuvgrtp.so.4
/usr/local/lib/libuvgrtp.so.4.0.1
```

The complete list is `stage/<arch>/.manifest/uvgrtp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa uvgrtp
ls dists/8.0/*/ | grep '^uvgrtp-'
```

This writes `uvgrtp-<ver>-r0.tar.gz` (run time) and `uvgrtp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add uvgrtp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/uvgrtp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/uvgrtp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_uvgrtp`](../../tests/src/) links uvgrtp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-uvgrtp`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-uvgrtp
/usr/local/bin/qnx-port-tests/test_uvgrtp
```

```
== uvgrtp ==
  ok   ...
PASS uvgrtp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_uvgrtp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_uvgrtp`](../../tests/src/) -- a program linked against uvgrtp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - version 4.0.1
   - session on 127.0.0.1
   - generic send and receive streams
   - RCC_SSRC set on the sender
   - 3 generic frames received
   - payloads intact (160 bytes each)
   - consecutive sequence numbers
   - timestamps 8000, 16000, 24000 as pushed
   - SSRC 0x0051a7e5 on every packet
   - RTP version 2, payload type 0 (PCMU/generic)
   - 9000-byte generic frame fragmented and reassembled intact
   - fragmenting generic streams
   - H.265 access unit pushed (4 NAL units, 6000-byte slice fragmented)
   - VPS, SPS, PPS and the 6000-byte IDR slice received intact, in order
   - H.265 streams

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | uvgrtp 4.0.1-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/uvgrtp-4.0.1-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | uvgrtp 4.0.1-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/uvgrtp-4.0.1-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.0.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add uvgrtp=<version>`), sources in `ported/uvgrtp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates uvgrtp`; port one with `scripts/port-update uvgrtp`.

<details><summary>recipe.sh</summary>

```bash
# uvgRTP: RTP library (RFC 3550) in C++ with RTCP, and payload formats for
# H.264, H.265, H.266 (VVC) and Opus, from Ultra Video Group.
VERSION=4.0.1; GIT=https://github.com/ultravideo/uvgRTP.git; REF=v4.0.1
BUILDSYS=cmake
# Shared library. SRTP/ZRTP need Crypto++, which is not ported: off. No unit
# tests (they download googletest) and no examples.
CMAKE_ARGS_EXTRA=( -DUVGRTP_BUILD_SHARED=ON -DUVGRTP_DISABLE_CRYPTO=ON -DUVGRTP_DISABLE_TESTS=ON
                   -DUVGRTP_DISABLE_EXAMPLES=ON -DUVGRTP_RELEASE_COMMIT=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
