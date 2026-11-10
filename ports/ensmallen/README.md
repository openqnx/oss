# ensmallen 3.11.1

> Header-only C++ numerical optimisation library on Armadillo

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/mlpack/ensmallen.git (tag `3.11.1`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`armadillo`](../armadillo/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_ensmallen`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `ensmallen-dev` -- header-only: there is no run-time package, the -dev package is the whole component (headers, pkg-config/CMake files).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`ensmallen-dev`](https://openqnx.github.io/oss/package/x86_64/ensmallen-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ensmallen-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add ensmallen-dev` (dependencies come along); one version: `apk add ensmallen-dev=<version>` | `apk del ensmallen-dev` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install ensmallen-dev` (dependencies first) | `oss-get remove ensmallen-dev` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-ensmallen && /usr/local/bin/qnx-port-tests/test_ensmallen    # its on-target test, from tests-8.0-<arch>
apk del ensmallen-dev qnx-test-ensmallen
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/ensmallen-dev.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/ensmallen-dev.list
rm var/lib/manual/ensmallen-dev.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ensmallen` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild armadillo
```

<details><summary>Full build chain (2)</summary>

[`armadillo`](../armadillo/) [`openblas`](../openblas/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.11.1 --recurse-submodules --shallow-submodules \
    https://github.com/mlpack/ensmallen.git src/ensmallen
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/ensmallen -B build/$QNX_ARCH/ensmallen -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DUSE_OPENMP=OFF \
      -DUSE_BANDICOOT=OFF \
      -DBUILD_TESTS=OFF
cmake --build build/$QNX_ARCH/ensmallen -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/ensmallen
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- ensmallen: header-only C++ library of numerical optimisers (L-BFGS, SGD variants, Adam, CMA-ES, simulated annealing, ...) on Armadillo matrices; mlpack's optimisation layer. Header-only: installs include/ensmallen* and the CMake package (ensmallen::ensmallen, which pulls in Armadillo).
- Armadillo is required at configure time (find_package) and by consumers.
- Header-only, so the package has no NEEDED entries to derive dependencies from: name what a consumer compiles and links against.
- OpenMP off: the SDP has no libgomp, and with USE_OPENMP on ensmallen defines ENS_USE_OPENMP for consumers even when OpenMP is not found. Bandicoot (GPU matrices) is not ported: off. Tests are not installed.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ensmallen`.

### 5. What it installs

305 files under `/usr/local`, 302 headers.

The complete list is `stage/<arch>/.manifest/ensmallen` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ensmallen
ls dists/8.0/*/ | grep '^ensmallen-'
```

This writes `ensmallen-<ver>-r0.tar.gz` (run time) and `ensmallen-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ensmallen
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ensmallen-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ensmallen-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_ensmallen`](../../tests/src/) links ensmallen the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-ensmallen`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-ensmallen
/usr/local/bin/qnx-port-tests/test_ensmallen
```

```
== ensmallen ==
  ok   ...
PASS ensmallen
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_ensmallen` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_ensmallen`](../../tests/src/) -- a program linked against ensmallen the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - L-BFGS: Rosenbrock minimum (1, 1)
   - callback saw the L-BFGS steps
   - gradient descent: (x-3)^2 + (y+1)^2 minimum (3, -1)
   - Adam: 4-D sphere function minimum 0 at the origin
   - simulated annealing: Booth minimum (1, 3)

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ensmallen 3.11.1-r1 | ✅ PASS | 2026-11-10 | [log](../../tests/logs/aarch64le/ensmallen-3.11.1-r1.log) |
| x86_64 | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ensmallen 3.11.1-r1 | ❌ FAIL | 2026-11-05 | [log](../../tests/logs/x86_64/ensmallen-3.11.1-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ensmallen 3.11.1-r1 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/ensmallen-3.11.1-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.11.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ensmallen=<version>`), sources in `ported/ensmallen/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ensmallen`; port one with `scripts/port-update ensmallen`.

<details><summary>recipe.sh</summary>

```bash
# ensmallen: header-only C++ library of numerical optimisers (L-BFGS, SGD
# variants, Adam, CMA-ES, simulated annealing, ...) on Armadillo matrices;
# mlpack's optimisation layer. Header-only: installs include/ensmallen* and
# the CMake package (ensmallen::ensmallen, which pulls in Armadillo).
VERSION=3.11.1; GIT=https://github.com/mlpack/ensmallen.git; REF=3.11.1
# Armadillo is required at configure time (find_package) and by consumers.
DEPS="armadillo"
# Header-only, so the package has no NEEDED entries to derive dependencies
# from: name what a consumer compiles and links against.
RUNTIME_DEPS="armadillo-dev"
BUILDSYS=cmake
# OpenMP off: the SDP has no libgomp, and with USE_OPENMP on ensmallen
# defines ENS_USE_OPENMP for consumers even when OpenMP is not found.
# Bandicoot (GPU matrices) is not ported: off. Tests are not installed.
CMAKE_ARGS_EXTRA=( -DUSE_OPENMP=OFF -DUSE_BANDICOOT=OFF -DBUILD_TESTS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
