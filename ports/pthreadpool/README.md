# pthreadpool 0-git-20250207

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/google/pthreadpool.git |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`fxdiv`](../fxdiv/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add pthreadpool
apk add pthreadpool-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install pthreadpool`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pthreadpool` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild fxdiv
```

### 2. Get the upstream source

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/google/pthreadpool.git src/pthreadpool
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/pthreadpool; B=build/$QNX_ARCH/pthreadpool; mkdir -p $B
```

```bash
do_install() { DESTDIR="$STAGE" cmake --install "$B" && rm -f "$STAGE$PREFIX/include/fxdiv.h"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Pinned to the commits XNNPACK (as ported by qnx-ports) builds with.
- pthreadpool builds fxdiv from FXDIV_SOURCE_DIR and, with it, installs fxdiv.h -- the fxdiv component's header. Leave that one to fxdiv.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pthreadpool`.

### 5. What it installs

2 files under `/usr/local`: 1 shared-library entries, 1 headers.

```
/usr/local/lib/libpthreadpool.so
```

The complete list is `stage/<arch>/.manifest/pthreadpool` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa pthreadpool
ls dists/8.0/*/ | grep '^pthreadpool-'
```

This writes `pthreadpool-<ver>-r0.tar.gz` (run time) and `pthreadpool-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add pthreadpool
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/pthreadpool-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/pthreadpool-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | pthreadpool 0_git20250207-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/pthreadpool-0_git20250207-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | pthreadpool 0_git20250207-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/pthreadpool-0_git20250207-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0-git-20250207**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add pthreadpool=<version>`), sources in `ported/pthreadpool/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates pthreadpool`; port one with `scripts/port-update pthreadpool`.

<details><summary>recipe.sh</summary>

```bash
# Pinned to the commits XNNPACK (as ported by qnx-ports) builds with.
VERSION=0-git-20250207; GIT=https://github.com/google/pthreadpool.git; COMMIT=f94ab76fe99754960035d520dce28e15b647e8cf
DEPS="fxdiv"
CMAKE_ARGS_EXTRA=( -DPTHREADPOOL_BUILD_TESTS=OFF -DPTHREADPOOL_BUILD_BENCHMARKS=OFF -DPTHREADPOOL_LIBRARY_TYPE=shared
                   -DFXDIV_SOURCE_DIR="$SRC_DIR/fxdiv" -DPTHREADPOOL_SYNC_PRIMITIVE=condvar )
# pthreadpool builds fxdiv from FXDIV_SOURCE_DIR and, with it, installs
# fxdiv.h -- the fxdiv component's header. Leave that one to fxdiv.
do_install() { DESTDIR="$STAGE" cmake --install "$B" && rm -f "$STAGE$PREFIX/include/fxdiv.h"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
