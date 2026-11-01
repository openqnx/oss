# pybind11 3.1.0

> Seamless operability between C++11 and Python (header-only)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/pybind/pybind11.git (tag `v3.1.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_pybind11`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `pybind11-dev` -- header-only: there is no run-time package, the -dev package is the whole component (headers, pkg-config/CMake files).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`pybind11-dev`](https://openqnx.github.io/oss/package/x86_64/pybind11-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/pybind11-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add pybind11-dev` (dependencies come along); one version: `apk add pybind11-dev=<version>` | `apk del pybind11-dev` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install pybind11-dev` (dependencies first) | `oss-get remove pybind11-dev` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-pybind11 && /usr/local/bin/qnx-port-tests/test_pybind11    # its on-target test, from tests-8.0-<arch>-m-z
apk del pybind11-dev qnx-test-pybind11
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/pybind11-dev.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/pybind11-dev.list
rm var/lib/manual/pybind11-dev.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pybind11` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v3.1.0 --recurse-submodules --shallow-submodules \
    https://github.com/pybind/pybind11.git src/pybind11
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/pybind11 -B build/$QNX_ARCH/pybind11 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DPYBIND11_TEST=OFF \
      -DPYBIND11_INSTALL=ON \
      -DPYBIND11_NOPYTHON=ON \
      -Dprefix_for_pc_file="$PREFIX"
cmake --build build/$QNX_ARCH/pybind11 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/pybind11
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Header-only: the headers and the CMake package (pybind11Config.cmake, pybind11_add_module) for building extension modules against the ported python3. Configuring does not need a Python: PYBIND11_NOPYTHON skips the interpreter probe, which in a cross build would find the build machine's. prefix_for_pc_file: with CMake >= 3.20 upstream derives the .pc prefix with cmake_path(RELATIVE_PATH) the wrong way round and writes an empty prefix= (so -I/include); name it.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pybind11`.

### 5. What it installs

64 files under `/usr/local`, 55 headers.

The complete list is `stage/<arch>/.manifest/pybind11` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa pybind11
ls dists/8.0/*/ | grep '^pybind11-'
```

This writes `pybind11-<ver>-r0.tar.gz` (run time) and `pybind11-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add pybind11
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/pybind11-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/pybind11-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_pybind11`](../../tests/src/) links pybind11 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-pybind11`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-pybind11
/usr/local/bin/qnx-port-tests/test_pybind11
```

```
== pybind11 ==
  ok   ...
PASS pybind11
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_pybind11` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_pybind11`](../../tests/src/) -- a program linked against pybind11 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - temporary directory
   - module written as qnxpb11.so
   - python3 imports the module and every check in it passes

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> pybind11`.

## Versions

This recipe builds **3.1.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add pybind11=<version>`), sources in `ported/pybind11/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates pybind11`; port one with `scripts/port-update pybind11`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.1.0; GIT=https://github.com/pybind/pybind11.git; REF=v3.1.0
# Header-only: the headers and the CMake package (pybind11Config.cmake,
# pybind11_add_module) for building extension modules against the ported
# python3. Configuring does not need a Python: PYBIND11_NOPYTHON skips the
# interpreter probe, which in a cross build would find the build machine's.
# prefix_for_pc_file: with CMake >= 3.20 upstream derives the .pc prefix with
# cmake_path(RELATIVE_PATH) the wrong way round and writes an empty prefix=
# (so -I/include); name it.
CMAKE_ARGS_EXTRA=( -DPYBIND11_TEST=OFF -DPYBIND11_INSTALL=ON -DPYBIND11_NOPYTHON=ON -Dprefix_for_pc_file="$PREFIX" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
