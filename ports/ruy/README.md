# ruy 0-git-20240321

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/google/ruy.git |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`cpuinfo`](../cpuinfo/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add ruy
apk add ruy-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install ruy`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ruy` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild cpuinfo
```

### 2. Get the upstream source

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/google/ruy.git src/ruy
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/ruy < ports/ruy/patches/0001-qnx-libregex-for-tests.patch
```

**[`0001-qnx-libregex-for-tests.patch`](patches/0001-qnx-libregex-for-tests.patch)** — add libregex dependency to test

> QNX: link ruy's test wrapper with libregex.
>
> test"). googletest uses regcomp()/regexec(), which QNX
> keeps in libregex rather than libc. The fork's other commits switch the
> cpuinfo and googletest submodules to qnx-ports' forks; this recipe builds
> against the cpuinfo port instead (RUY_FIND_CPUINFO=ON), so they are left out.

Touches: `ruy/CMakeLists.txt` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/ruy -B build/$QNX_ARCH/ruy -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DRUY_MINIMAL_BUILD=ON \
      -DRUY_FIND_CPUINFO=ON \
      -DBUILD_SHARED_LIBS=ON \
      -DRUY_ENABLE_INSTALL=ON
cmake --build build/$QNX_ARCH/ruy -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/ruy
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ruy`.

### 5. What it installs

82 files under `/usr/local`, 49 headers.

The complete list is `stage/<arch>/.manifest/ruy` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ruy
ls dists/8.0/*/ | grep '^ruy-'
```

This writes `ruy-<ver>-r0.tar.gz` (run time) and `ruy-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ruy
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ruy-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ruy-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ruy 0_git20240321-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/ruy-0_git20240321-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ruy 0_git20240321-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/ruy-0_git20240321-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0-git-20240321**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ruy=<version>`), sources in `ported/ruy/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ruy`; port one with `scripts/port-update ruy`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0-git-20240321; GIT=https://github.com/google/ruy.git; COMMIT=c08ec529fc91722bde519628d9449258082eb847
DEPS="cpuinfo"
CMAKE_ARGS_EXTRA=( -DRUY_MINIMAL_BUILD=ON -DRUY_FIND_CPUINFO=ON -DBUILD_SHARED_LIBS=ON -DRUY_ENABLE_INSTALL=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
