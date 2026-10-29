# kineto 0-git-20250214

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/pytorch/kineto.git |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install kineto
oss-get install kineto-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild kineto` performs exactly the same steps (2–5) in one command.

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
    https://github.com/pytorch/kineto.git src/kineto
```

The project lives in the `libkineto/` subdirectory of the repository.

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/kineto < ports/kineto/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Support QNX.

> Imported from qnx-ports/kineto (a054a4be0db1...qnx_main); original authorship kept.
>
>
> ---
>  libkineto/src/ThreadUtil.cpp | 6 ++++++
>  1 file changed, 6 insertions(+)

Touches: `libkineto/src/ThreadUtil.cpp` · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/kineto/libkineto -B build/$QNX_ARCH/kineto -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DKINETO_BUILD_TESTS=OFF \
      -DLIBKINETO_NOCUPTI=ON \
      -DLIBKINETO_NOROCTRACER=ON \
      -DLIBKINETO_NOXPUPTI=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DPython3_EXECUTABLE=/usr/bin/python3 \
      -DFMT_INSTALL=OFF
cmake --build build/$QNX_ARCH/kineto -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/kineto
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Kineto: PyTorch's CPU/GPU profiling library (libkineto), at the upstream commit qnx-ports' QNX port is based on. Built standalone for CPU tracing; patches/: QNX build support.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH kineto`.

### 5. What it installs

19 files under `/usr/local`, 16 headers.

The complete list is `stage/<arch>/.manifest/kineto` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH kineto
ls repo/8.0/*/ | grep '^kineto-'
```

This writes `kineto-<ver>-r0.tar.gz` (run time) and `kineto-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install kineto
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/kineto-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/kineto-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# Kineto: PyTorch's CPU/GPU profiling library (libkineto), at the upstream
# commit qnx-ports' QNX port is based on. Built standalone for CPU tracing;
# patches/: QNX build support.
VERSION=0-git-20250214; GIT=https://github.com/pytorch/kineto.git; COMMIT=a054a4be0db117c579a21747debf19c863631f26
SUBDIR=libkineto
CMAKE_ARGS_EXTRA=( -DKINETO_BUILD_TESTS=OFF -DLIBKINETO_NOCUPTI=ON -DLIBKINETO_NOROCTRACER=ON -DLIBKINETO_NOXPUPTI=ON
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DPython3_EXECUTABLE=/usr/bin/python3
                   -DFMT_INSTALL=OFF )
# kineto compiles its bundled fmt (third_party/fmt, header-only) and, left to
# itself, installs it: an older fmt's headers, CMake package, fmt.pc and
# libfmt.a over the fmt port's, after which every fmt user failed to link
# ("undefined reference to fmt::v10::vformat"). Keep the bundled copy private.
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
