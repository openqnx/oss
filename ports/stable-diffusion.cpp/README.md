# stable-diffusion.cpp 0_git20260601

> Stable Diffusion and FLUX image generation inference in C/C++

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/leejet/stable-diffusion.cpp.git |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_sdcpp`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `stable-diffusion.cpp` (programs, shared libraries, run-time data) and `stable-diffusion.cpp-dev` (headers, pkg-config/CMake files, static libraries; needs `stable-diffusion.cpp`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`stable-diffusion.cpp`](https://openqnx.github.io/oss/package/x86_64/stable-diffusion.cpp.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/stable-diffusion.cpp.html)) · [`stable-diffusion.cpp-dev`](https://openqnx.github.io/oss/package/x86_64/stable-diffusion.cpp-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/stable-diffusion.cpp-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add stable-diffusion.cpp` (dependencies come along); one version: `apk add stable-diffusion.cpp=<version>` | `apk del stable-diffusion.cpp` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install stable-diffusion.cpp` (dependencies first) | `oss-get remove stable-diffusion.cpp` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`stable-diffusion.cpp-dev` the same way, by its own name (`apk add stable-diffusion.cpp-dev`, `oss-get install stable-diffusion.cpp-dev`, by hand with its own file and list `stable-diffusion.cpp-dev.list`); remove it before `stable-diffusion.cpp`, which it needs.

```sh
# on the QNX target, as root
apk add stable-diffusion.cpp stable-diffusion.cpp-dev
apk add qnx-test-sdcpp && /usr/local/bin/qnx-port-tests/test_sdcpp    # its on-target test
apk del stable-diffusion.cpp stable-diffusion.cpp-dev qnx-test-sdcpp
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/stable-diffusion.cpp.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/stable-diffusion.cpp.list
rm var/lib/manual/stable-diffusion.cpp.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild stable-diffusion.cpp` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/leejet/stable-diffusion.cpp.git src/stable-diffusion.cpp
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/stable-diffusion.cpp < ports/stable-diffusion.cpp/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — stable-diffusion.cpp on QNX, from QNX's port (github.com/qnx-ports/

> stable-diffusion.cpp, branch qnx-8.0): the server example links QNX's
> libsocket, and the shared library takes all of ggml-vulkan's objects when
> that backend is built (not here: the SDP has no Vulkan).

Touches: `CMakeLists.txt`, `examples/server/CMakeLists.txt` · Upstream status: Inappropriate [QNX's port, kept as QNX published it]

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/stable-diffusion.cpp; B=build/$QNX_ARCH/stable-diffusion.cpp; mkdir -p $B
```

```bash
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  local p; mkdir -p "$STAGE$PREFIX/bin"
  for p in sd-cli sd-server; do [ -e "$STAGE$OPT/bin/$p" ] && ln -sf "../opt/stable-diffusion.cpp/bin/$p" "$STAGE$PREFIX/bin/$p"; done; return 0
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- stable-diffusion.cpp: Stable Diffusion / FLUX image generation in C/C++ on ggml, CPU inference (the sd command and the library), with QNX's changes (patches/0001, from github.com/qnx-ports/stable-diffusion.cpp; the commit QNX's port is based on).
- It carries its own ggml (libggml*.so, ggml.h), as stable-diffusion.cpp and whisper.cpp each do, in versions that are not interchangeable: each gets a prefix of its own, /usr/local/opt/stable-diffusion.cpp, its commands linked into bin/. The run path is that prefix's lib/, absolute: QNX's loader does not expand $ORIGIN (docs/porting-hazards.md).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH stable-diffusion.cpp`.

### 5. What it installs

29 files under `/usr/local`, 2 programs.

```
/usr/local/bin/sd-cli
/usr/local/bin/sd-server
```

The complete list is `stage/<arch>/.manifest/stable-diffusion.cpp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa stable-diffusion.cpp
ls dists/8.0/*/ | grep '^stable-diffusion.cpp-'
```

This writes `stable-diffusion.cpp-<ver>-r0.tar.gz` (run time) and `stable-diffusion.cpp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add stable-diffusion.cpp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/stable-diffusion.cpp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/stable-diffusion.cpp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_sdcpp`](../../tests/src/) links stable-diffusion.cpp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-sdcpp`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-sdcpp
/usr/local/bin/qnx-port-tests/test_sdcpp
```

```
== stable-diffusion.cpp ==
  ok   ...
PASS stable-diffusion.cpp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_sdcpp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_sdcpp`](../../tests/src/) -- a program linked against stable-diffusion.cpp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | stable-diffusion.cpp 0_git20260601-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/stable-diffusion.cpp-0_git20260601-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | stable-diffusion.cpp 0_git20260601-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/stable-diffusion.cpp-0_git20260601-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0_git20260601**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add stable-diffusion.cpp=<version>`), sources in `ported/stable-diffusion.cpp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates stable-diffusion.cpp`; port one with `scripts/port-update stable-diffusion.cpp`.

<details><summary>recipe.sh</summary>

```bash
# stable-diffusion.cpp: Stable Diffusion / FLUX image generation in C/C++ on
# ggml, CPU inference (the sd command and the library), with QNX's changes
# (patches/0001, from github.com/qnx-ports/stable-diffusion.cpp; the commit
# QNX's port is based on).
VERSION=0_git20260601; GIT=https://github.com/leejet/stable-diffusion.cpp.git; COMMIT=7948df8ac1070f5f6881b8d34675821893eb97d6
DEPS=""
BUILDSYS=cmake
# It carries its own ggml (libggml*.so, ggml.h), as stable-diffusion.cpp and
# whisper.cpp each do, in versions that are not interchangeable: each gets a
# prefix of its own, /usr/local/opt/stable-diffusion.cpp, its commands linked into bin/.
# The run path is that prefix's lib/, absolute: QNX's loader does not expand
# $ORIGIN (docs/porting-hazards.md).
OPT="$PREFIX/opt/stable-diffusion.cpp"
CMAKE_ARGS_EXTRA=( -DCMAKE_INSTALL_PREFIX="$OPT" -DCMAKE_INSTALL_RPATH="$OPT/lib" -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON -DSD_BUILD_SHARED_LIBS=ON -DSD_BUILD_EXAMPLES=ON -DGGML_NATIVE=OFF -DGGML_OPENMP=OFF -DGGML_VULKAN=OFF )
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  local p; mkdir -p "$STAGE$PREFIX/bin"
  for p in sd-cli sd-server; do [ -e "$STAGE$OPT/bin/$p" ] && ln -sf "../opt/stable-diffusion.cpp/bin/$p" "$STAGE$PREFIX/bin/$p"; done; return 0
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
