# flac 1.4.3

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/xiph/flac.git (tag `1.4.3`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`libogg`](../libogg/) |
| Run-time dependencies | [`libogg`](../libogg/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_flac`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install flac
oss-get install flac-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild flac` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libogg
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.4.3 --recurse-submodules --shallow-submodules \
    https://github.com/xiph/flac.git src/flac
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/flac -B build/$QNX_ARCH/flac -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_CXXLIBS=ON \
      -DBUILD_PROGRAMS=OFF \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_TESTING=OFF \
      -DBUILD_DOCS=OFF \
      -DWITH_OGG=ON \
      -DOGG_ROOT="$STAGE$PREFIX" \
      -DINSTALL_MANPAGES=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/flac -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/flac
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH flac`.

### 5. What it installs

26 files under `/usr/local`: 6 shared-library entries, 14 headers.

```
/usr/local/lib/libFLAC++.so
/usr/local/lib/libFLAC++.so.10
/usr/local/lib/libFLAC++.so.10.0.1
/usr/local/lib/libFLAC.so
/usr/local/lib/libFLAC.so.12
/usr/local/lib/libFLAC.so.12.1.0
```

The complete list is `stage/<arch>/.manifest/flac` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH flac
ls repo/8.0/*/ | grep '^flac-'
```

This writes `flac-<ver>-r0.tar.gz` (run time) and `flac-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install flac
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/flac-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/flac-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_flac`](../../tests/src/) links flac the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_flac root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_flac'
```

```
== flac ==
  ok   ...
PASS flac
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.4.3; GIT=https://github.com/xiph/flac.git; REF=1.4.3
DEPS="libogg"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_CXXLIBS=ON -DBUILD_PROGRAMS=OFF
                   -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF -DBUILD_DOCS=OFF
                   -DWITH_OGG=ON -DOGG_ROOT="$STAGE$PREFIX"
                   -DINSTALL_MANPAGES=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
