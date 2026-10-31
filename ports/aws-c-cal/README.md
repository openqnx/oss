# aws-c-cal 0.9.13

> AWS crypto abstraction layer, C wrapper for cryptography primitives

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/awslabs/aws-c-cal.git (tag `v0.9.13`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`aws-c-common`](../aws-c-common/) [`openssl`](../openssl/) |
| Run-time dependencies | [`aws-c-common`](../aws-c-common/) [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_aws_c_cal`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `aws-c-cal` (programs, shared libraries, run-time data) and `aws-c-cal-dev` (headers, pkg-config/CMake files, static libraries; needs `aws-c-cal`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`aws-c-cal`](https://openqnx.github.io/oss/package/x86_64/aws-c-cal.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/aws-c-cal.html)) · [`aws-c-cal-dev`](https://openqnx.github.io/oss/package/x86_64/aws-c-cal-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/aws-c-cal-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add aws-c-cal` (dependencies come along); one version: `apk add aws-c-cal=<version>` | `apk del aws-c-cal` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install aws-c-cal` (dependencies first) | `oss-get remove aws-c-cal` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`aws-c-cal-dev` the same way, by its own name (`apk add aws-c-cal-dev`, `oss-get install aws-c-cal-dev`, by hand with its own file and list `aws-c-cal-dev.list`); remove it before `aws-c-cal`, which it needs.

```sh
# on the QNX target, as root
apk add aws-c-cal aws-c-cal-dev
apk add qnx-test-aws-c-cal && /usr/local/bin/qnx-port-tests/test_aws_c_cal    # its on-target test
apk del aws-c-cal aws-c-cal-dev qnx-test-aws-c-cal
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/aws-c-cal.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/aws-c-cal.list
rm var/lib/manual/aws-c-cal.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild aws-c-cal` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild aws-c-common openssl
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.9.13 --recurse-submodules --shallow-submodules \
    https://github.com/awslabs/aws-c-cal.git src/aws-c-cal
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/aws-c-cal < ports/aws-c-cal/patches/0001-hkdf-include-guard.patch
```

**[`0001-hkdf-include-guard.patch`](patches/0001-hkdf-include-guard.patch)** — Give aws/cal/hkdf.h an include guard of its own.

> hkdf.h reused ecc.h's guard (AWS_CAL_ECC_H), so a source that includes
> both headers silently loses whichever comes second: with hkdf.h first,
> none of the ECC API is declared; with ecc.h first, aws_hkdf_derive and
> HKDF_HMAC_SHA512 are missing.

Touches: `include/aws/cal/hkdf.h` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/aws-c-cal -B build/$QNX_ARCH/aws-c-cal -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" \
      -DAWS_WARNINGS_ARE_ERRORS=OFF \
      -DUSE_OPENSSL=ON
cmake --build build/$QNX_ARCH/aws-c-cal -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/aws-c-cal
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The release aws-crt-cpp v0.38.5 pins as its submodule, so the whole AWS C runtime is one consistent set.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH aws-c-cal`.

### 5. What it installs

15 files under `/usr/local`: 2 shared-library entries, 9 headers.

```
/usr/local/lib/libaws-c-cal.so
/usr/local/lib/libaws-c-cal.so.1.0.0
```

The complete list is `stage/<arch>/.manifest/aws-c-cal` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa aws-c-cal
ls dists/8.0/*/ | grep '^aws-c-cal-'
```

This writes `aws-c-cal-<ver>-r0.tar.gz` (run time) and `aws-c-cal-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add aws-c-cal
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/aws-c-cal-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/aws-c-cal-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_aws_c_cal`](../../tests/src/) links aws-c-cal the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-aws-c-cal`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-aws-c-cal
/usr/local/bin/qnx-port-tests/test_aws_c_cal
```

```
== aws-c-cal ==
  ok   ...
PASS aws-c-cal
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_aws_c_cal` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_aws_c_cal`](../../tests/src/) -- a program linked against aws-c-cal the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - SHA-256(\"abc\")
   - SHA-1(\"abc\")
   - MD5(\"abc\")
   - streaming SHA-256
   - HMAC-SHA256 (RFC 4231 #2)
   - AES-256-CBC cipher created
   - AES-256-CBC NIST vector
   - AES-256-CBC decrypts back to the plaintext
   - HKDF-SHA512 (RFC 5869 case 1 inputs, Python-computed answer)
   - generate an ECDSA P-256 key pair
   - ECDSA sign
   - ECDSA signature verifies
   - tampered digest is rejected
   - P-256 public key coordinates are 32 bytes

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aws-c-cal 0.9.13-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/aws-c-cal-0.9.13-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aws-c-cal 0.9.13-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/aws-c-cal-0.9.13-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aws-c-cal 0.9.13-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/aws-c-cal-0.9.13-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aws-c-cal 0.9.13-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/aws-c-cal-0.9.13-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.9.13**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add aws-c-cal=<version>`), sources in `ported/aws-c-cal/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates aws-c-cal`; port one with `scripts/port-update aws-c-cal`.

<details><summary>recipe.sh</summary>

```bash
# The release aws-crt-cpp v0.38.5 pins as its submodule, so the
# whole AWS C runtime is one consistent set.
VERSION=0.9.13; GIT=https://github.com/awslabs/aws-c-cal.git; REF=v0.9.13
DEPS="aws-c-common openssl"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" -DAWS_WARNINGS_ARE_ERRORS=OFF -DUSE_OPENSSL=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
