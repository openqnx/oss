# s2n-tls 1.7.0

> Small, fast C99 implementation of the TLS/SSL protocols from AWS

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/aws/s2n-tls.git (tag `v1.7.0`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`openssl`](../openssl/) |
| Run-time dependencies | [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_s2n_tls`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `s2n-tls` (programs, shared libraries, run-time data) and `s2n-tls-dev` (headers, pkg-config/CMake files, static libraries; needs `s2n-tls`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`s2n-tls`](https://openqnx.github.io/oss/package/x86_64/s2n-tls.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/s2n-tls.html)) · [`s2n-tls-dev`](https://openqnx.github.io/oss/package/x86_64/s2n-tls-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/s2n-tls-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add s2n-tls` (dependencies come along); one version: `apk add s2n-tls=<version>` | `apk del s2n-tls` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install s2n-tls` (dependencies first) | `oss-get remove s2n-tls` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`s2n-tls-dev` the same way, by its own name (`apk add s2n-tls-dev`, `oss-get install s2n-tls-dev`, by hand with its own file and list `s2n-tls-dev.list`); remove it before `s2n-tls`, which it needs.

```sh
# on the QNX target, as root
apk add s2n-tls s2n-tls-dev
apk add qnx-test-s2n-tls && /usr/local/bin/qnx-port-tests/test_s2n_tls    # its on-target test, from tests-8.0-<arch>-m-z
apk del s2n-tls s2n-tls-dev qnx-test-s2n-tls
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/s2n-tls.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/s2n-tls.list
rm var/lib/manual/s2n-tls.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild s2n-tls` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v1.7.0 --recurse-submodules --shallow-submodules \
    https://github.com/aws/s2n-tls.git src/s2n-tls
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/s2n-tls < ports/s2n-tls/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add QNX Support

> Imported from qnx-ports/s2n-tls (v1.5.25...qnx-v1.5.25).
>
> ---
>  CMakeLists.txt                                 |  2 +-
>  crypto/s2n_drbg.c                              |  3 +++
>  stuffer/s2n_stuffer.c                          |  5 +++++
>  tls/extensions/s2n_client_psk.c                |  4 ++++
>  tls/extensions/s2n_client_supported_versions.c |  4 ++++
>  tls/extensions/s2n_supported_versions.c        |  4 ++++
>  tls/s2n_alerts.c                               |  4 ++++
>  tls/s2n_cbc.c                                  |  5 +++++
>  tls/s2n_client_hello.c                         |  4 ++++
>  tls/s2n_client_key_exchange.c                  |  4 ++++
>  tls/s2n_config.h                               |  4 ++++
>  tls/s2n_connection.c                           |  3 +++
>  tls/s2n_early_data.c                           |  4 ++++
>  tls/s2n_early_data_io.c                        |  4 ++++
>  tls/s2n_fingerprint_ja4.c                      |  3 +++
>  tls/s2n_handshake_io.c                         |  4 ++++
>  tls/s2n_ktls_io.c                              | 10 ++++++++++
>  tls/s2n_post_handshake.c                       |  4 ++++
>  tls/s2n_prf.c                                  |  4 ++++
>  tls/s2n_psk.c                                  |  4 ++++
>  tls/s2n_record_read.c                          |  4 ++++
>  tls/s2n_record_write.c                         |  5 +++++
>  tls/s2n_recv.c                                 |  5 +++++
>  tls/s2n_resume.c                               |  4 ++++
>  tls/s2n_send.c                                 |  4 ++++
>  tls/s2n_server_hello.c                         |  4 ++++
>  tls/s2n_server_new_session_ticket.c            |  4 ++++
>  tls/s2n_tls13_handshake.c                      |  4 ++++
>  utils/s2n_array.c                              |  4 ++++
>  utils/s2n_blob.c                               |  4 ++++
>  utils/s2n_mem.c                                |  4 ++++
>  utils/s2n_random.c                             |  3 +++
>  32 files changed, 131 insertions(+), 1 deletion(-)

Touches: `CMakeLists.txt`, `crypto/s2n_drbg.c`, `stuffer/s2n_stuffer.c`, `tls/extensions/s2n_client_psk.c`, `tls/extensions/s2n_client_supported_versions.c`, `tls/extensions/s2n_supported_versions.c` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/s2n-tls -B build/$QNX_ARCH/s2n-tls -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DUNSAFE_TREAT_WARNINGS_AS_ERRORS=OFF \
      -DBUILD_TESTING=OFF
cmake --build build/$QNX_ARCH/s2n-tls -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/s2n-tls
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The release aws-crt-cpp v0.38.5 pins as its submodule.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH s2n-tls`.

### 5. What it installs

18 files under `/usr/local`: 3 shared-library entries, 11 headers.

```
/usr/local/lib/libs2n.so
/usr/local/lib/libs2n.so.1
/usr/local/lib/libs2n.so.1.0.0
```

The complete list is `stage/<arch>/.manifest/s2n-tls` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa s2n-tls
ls dists/8.0/*/ | grep '^s2n-tls-'
```

This writes `s2n-tls-<ver>-r0.tar.gz` (run time) and `s2n-tls-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add s2n-tls
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/s2n-tls-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/s2n-tls-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_s2n_tls`](../../tests/src/) links s2n-tls the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-s2n-tls`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-s2n-tls
/usr/local/bin/qnx-port-tests/test_s2n_tls
```

```
== s2n-tls ==
  ok   ...
PASS s2n-tls
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_s2n_tls` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_s2n_tls`](../../tests/src/) -- a program linked against s2n-tls the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - s2n_init
   - certificate chain and ECDSA key loaded from PEM
   - server policy default_tls13
   - server certificate installed
   - client trusts the self-signed certificate
   - listen on 127.0.0.1
   - TCP connect
   - TLS handshake completes, peer certificate verified
   - negotiated TLS 1.3
   - an AEAD cipher suite
   - client s2n_send
   - server echo received over TLS
   - untrusted certificate is rejected
   - server decrypted the client's data
   - s2n_cleanup

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | s2n-tls 1.7.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/s2n-tls-1.7.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | s2n-tls 1.7.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/s2n-tls-1.7.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | s2n-tls 1.7.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/s2n-tls-1.7.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | s2n-tls 1.7.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/s2n-tls-1.7.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.7.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add s2n-tls=<version>`), sources in `ported/s2n-tls/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates s2n-tls`; port one with `scripts/port-update s2n-tls`.

<details><summary>recipe.sh</summary>

```bash
# The release aws-crt-cpp v0.38.5 pins as its submodule.
VERSION=1.7.0; GIT=https://github.com/aws/s2n-tls.git; REF=v1.7.0
DEPS="openssl"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DUNSAFE_TREAT_WARNINGS_AS_ERRORS=OFF -DBUILD_TESTING=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
