# aws-c-cal 0.9.13

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/awslabs/aws-c-cal.git (tag `v0.9.13`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`aws-c-common`](../aws-c-common/) [`openssl`](../openssl/) |
| Run-time dependencies | [`aws-c-common`](../aws-c-common/) [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add aws-c-cal
apk add aws-c-cal-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install aws-c-cal`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

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

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

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
