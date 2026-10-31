# aws-c-s3 0.12.2

> Asynchronous high-throughput AWS S3 client library in C

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/awslabs/aws-c-s3.git (tag `v0.12.2`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`aws-c-auth`](../aws-c-auth/) [`aws-checksums`](../aws-checksums/) |
| Run-time dependencies | [`aws-c-auth`](../aws-c-auth/) [`aws-c-cal`](../aws-c-cal/) [`aws-c-common`](../aws-c-common/) [`aws-c-compression`](../aws-c-compression/) [`aws-c-http`](../aws-c-http/) [`aws-c-io`](../aws-c-io/) [`aws-c-sdkutils`](../aws-c-sdkutils/) [`aws-checksums`](../aws-checksums/) [`openssl`](../openssl/) [`s2n-tls`](../s2n-tls/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_aws_c_s3`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `aws-c-s3` (programs, shared libraries, run-time data) and `aws-c-s3-dev` (headers, pkg-config/CMake files, static libraries; needs `aws-c-s3`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`aws-c-s3`](https://openqnx.github.io/oss/package/x86_64/aws-c-s3.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/aws-c-s3.html)) · [`aws-c-s3-dev`](https://openqnx.github.io/oss/package/x86_64/aws-c-s3-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/aws-c-s3-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add aws-c-s3` (dependencies come along); one version: `apk add aws-c-s3=<version>` | `apk del aws-c-s3` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install aws-c-s3` (dependencies first) | `oss-get remove aws-c-s3` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`aws-c-s3-dev` the same way, by its own name (`apk add aws-c-s3-dev`, `oss-get install aws-c-s3-dev`, by hand with its own file and list `aws-c-s3-dev.list`); remove it before `aws-c-s3`, which it needs.

```sh
# on the QNX target, as root
apk add aws-c-s3 aws-c-s3-dev
apk add qnx-test-aws-c-s3 && /usr/local/bin/qnx-port-tests/test_aws_c_s3    # its on-target test
apk del aws-c-s3 aws-c-s3-dev qnx-test-aws-c-s3
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/aws-c-s3.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/aws-c-s3.list
rm var/lib/manual/aws-c-s3.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild aws-c-s3` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild aws-c-auth aws-checksums
```

<details><summary>Full build chain (10)</summary>

[`aws-c-auth`](../aws-c-auth/) [`aws-c-cal`](../aws-c-cal/) [`aws-c-common`](../aws-c-common/) [`aws-c-compression`](../aws-c-compression/) [`aws-c-http`](../aws-c-http/) [`aws-c-io`](../aws-c-io/) [`aws-c-sdkutils`](../aws-c-sdkutils/) [`aws-checksums`](../aws-checksums/) [`openssl`](../openssl/) [`s2n-tls`](../s2n-tls/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.12.2 --recurse-submodules --shallow-submodules \
    https://github.com/awslabs/aws-c-s3.git src/aws-c-s3
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/aws-c-s3 -B build/$QNX_ARCH/aws-c-s3 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" \
      -DAWS_WARNINGS_ARE_ERRORS=OFF
cmake --build build/$QNX_ARCH/aws-c-s3 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/aws-c-s3
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The release aws-crt-cpp v0.38.5 pins as its submodule, so the whole AWS C runtime is one consistent set.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH aws-c-s3`.

### 5. What it installs

12 files under `/usr/local`: 3 shared-library entries, 6 headers.

```
/usr/local/lib/libaws-c-s3.so
/usr/local/lib/libaws-c-s3.so.0unstable
/usr/local/lib/libaws-c-s3.so.1.0.0
```

The complete list is `stage/<arch>/.manifest/aws-c-s3` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa aws-c-s3
ls dists/8.0/*/ | grep '^aws-c-s3-'
```

This writes `aws-c-s3-<ver>-r0.tar.gz` (run time) and `aws-c-s3-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add aws-c-s3
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/aws-c-s3-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/aws-c-s3-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_aws_c_s3`](../../tests/src/) links aws-c-s3 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-aws-c-s3`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-aws-c-s3
/usr/local/bin/qnx-port-tests/test_aws_c_s3
```

```
== aws-c-s3 ==
  ok   ...
PASS aws-c-s3
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_aws_c_s3` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_aws_c_s3`](../../tests/src/) -- a program linked against aws-c-s3 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - S3 stand-in listening on 127.0.0.1
   - S3 client created (TLS off, SigV4 with test keys)
   - GetObject meta request finished
   - 200 OK, no error
   - object body delivered through body_callback
   - server saw the request line
   - request arrived SigV4-signed (Authorization: AWS4-HMAC-SHA256 ...)
   - second meta request finished
   - 404 reported as a failed meta request
   - error response body handed to the caller
   - client shut down cleanly

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aws-c-s3 0.12.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/aws-c-s3-0.12.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aws-c-s3 0.12.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/aws-c-s3-0.12.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aws-c-s3 0.12.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/aws-c-s3-0.12.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aws-c-s3 0.12.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/aws-c-s3-0.12.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.12.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add aws-c-s3=<version>`), sources in `ported/aws-c-s3/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates aws-c-s3`; port one with `scripts/port-update aws-c-s3`.

<details><summary>recipe.sh</summary>

```bash
# The release aws-crt-cpp v0.38.5 pins as its submodule, so the
# whole AWS C runtime is one consistent set.
VERSION=0.12.2; GIT=https://github.com/awslabs/aws-c-s3.git; REF=v0.12.2
DEPS="aws-c-auth aws-checksums"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" -DAWS_WARNINGS_ARE_ERRORS=OFF  )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
