# aws-c-io 0.26.1

> Event-driven I/O and TLS library for the AWS SDK for C

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/awslabs/aws-c-io.git (tag `v0.26.1`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`aws-c-common`](../aws-c-common/) [`aws-c-cal`](../aws-c-cal/) [`s2n-tls`](../s2n-tls/) |
| Run-time dependencies | [`aws-c-cal`](../aws-c-cal/) [`aws-c-common`](../aws-c-common/) [`openssl`](../openssl/) [`s2n-tls`](../s2n-tls/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_aws_c_io`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `aws-c-io` (programs, shared libraries, run-time data) and `aws-c-io-dev` (headers, pkg-config/CMake files, static libraries; needs `aws-c-io`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`aws-c-io`](https://openqnx.github.io/oss/package/x86_64/aws-c-io.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/aws-c-io.html)) · [`aws-c-io-dev`](https://openqnx.github.io/oss/package/x86_64/aws-c-io-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/aws-c-io-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add aws-c-io` (dependencies come along); one version: `apk add aws-c-io=<version>` | `apk del aws-c-io` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install aws-c-io` (dependencies first) | `oss-get remove aws-c-io` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`aws-c-io-dev` the same way, by its own name (`apk add aws-c-io-dev`, `oss-get install aws-c-io-dev`, by hand with its own file and list `aws-c-io-dev.list`); remove it before `aws-c-io`, which it needs.

```sh
# on the QNX target, as root
apk add aws-c-io aws-c-io-dev
apk add qnx-test-aws-c-io && /usr/local/bin/qnx-port-tests/test_aws_c_io    # its on-target test
apk del aws-c-io aws-c-io-dev qnx-test-aws-c-io
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/aws-c-io.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/aws-c-io.list
rm var/lib/manual/aws-c-io.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild aws-c-io` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (4 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild aws-c-common aws-c-cal s2n-tls
```

<details><summary>Full build chain (4)</summary>

[`aws-c-cal`](../aws-c-cal/) [`aws-c-common`](../aws-c-common/) [`openssl`](../openssl/) [`s2n-tls`](../s2n-tls/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.26.1 --recurse-submodules --shallow-submodules \
    https://github.com/awslabs/aws-c-io.git src/aws-c-io
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/aws-c-io < ports/aws-c-io/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add QNX Support

> Imported from qnx-ports/aws-c-io (v0.26.1...qnx-v0.26.1).
>
> ---
>  CMakeLists.txt                       | 14 +++++++++++++-
>  source/linux/epoll_event_loop.c      |  6 ++++++
>  source/posix/host_resolver.c         |  4 ++--
>  source/posix/socket.c                |  7 +++++--
>  source/s2n/s2n_tls_channel_handler.c | 16 ++++++++++++++--
>  5 files changed, 40 insertions(+), 7 deletions(-)

Touches: `CMakeLists.txt`, `source/linux/epoll_event_loop.c`, `source/posix/host_resolver.c`, `source/posix/socket.c`, `source/s2n/s2n_tls_channel_handler.c` · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/aws-c-io -B build/$QNX_ARCH/aws-c-io -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" \
      -DAWS_WARNINGS_ARE_ERRORS=OFF
cmake --build build/$QNX_ARCH/aws-c-io -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/aws-c-io
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The release aws-crt-cpp v0.38.5 pins as its submodule, so the whole AWS C runtime is one consistent set.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH aws-c-io`.

### 5. What it installs

30 files under `/usr/local`: 2 shared-library entries, 25 headers.

```
/usr/local/lib/libaws-c-io.so
/usr/local/lib/libaws-c-io.so.1.0.0
```

The complete list is `stage/<arch>/.manifest/aws-c-io` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa aws-c-io
ls dists/8.0/*/ | grep '^aws-c-io-'
```

This writes `aws-c-io-<ver>-r0.tar.gz` (run time) and `aws-c-io-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add aws-c-io
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/aws-c-io-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/aws-c-io-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_aws_c_io`](../../tests/src/) links aws-c-io the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-aws-c-io`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-aws-c-io
/usr/local/bin/qnx-port-tests/test_aws_c_io
```

```
== aws-c-io ==
  ok   ...
PASS aws-c-io
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_aws_c_io` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_aws_c_io`](../../tests/src/) -- a program linked against aws-c-io the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - event loop group with one loop
   - immediate and 20 ms tasks both ran
   - tasks run on the event loop thread
   - the immediate task ran before the delayed one
   - pipe created on the loop
   - bytes written to the pipe arrive through the readable callback
   - listener socket created
   - bind to an ephemeral loopback port and listen
   - accepting on the event loop
   - connect started
   - server received \"ping\"
   - client received \"pong\"
   - sockets closed on the loop
   - input stream length and first read
   - seek from end, then EOF
   - two PEM objects parsed around junk
   - certificate decoded
   - private key decoded

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aws-c-io 0.26.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/aws-c-io-0.26.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aws-c-io 0.26.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/aws-c-io-0.26.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aws-c-io 0.26.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/aws-c-io-0.26.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aws-c-io 0.26.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/aws-c-io-0.26.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.26.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add aws-c-io=<version>`), sources in `ported/aws-c-io/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates aws-c-io`; port one with `scripts/port-update aws-c-io`.

<details><summary>recipe.sh</summary>

```bash
# The release aws-crt-cpp v0.38.5 pins as its submodule, so the
# whole AWS C runtime is one consistent set.
VERSION=0.26.1; GIT=https://github.com/awslabs/aws-c-io.git; REF=v0.26.1
DEPS="aws-c-common aws-c-cal s2n-tls"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" -DAWS_WARNINGS_ARE_ERRORS=OFF  )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
