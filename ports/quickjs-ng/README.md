# quickjs-ng 0.8.0

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/quickjs-ng/quickjs.git (tag `v0.8.0`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_quickjs`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install quickjs-ng
oss-get install quickjs-ng-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild quickjs-ng` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v0.8.0 --recurse-submodules --shallow-submodules \
    https://github.com/quickjs-ng/quickjs.git src/quickjs-ng
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/quickjs-ng < ports/quickjs-ng/patches/0001-qnx-sighandler-t-and-test262.patch
```

**[`0001-qnx-sighandler-t-and-test262.patch`](patches/0001-qnx-sighandler-t-and-test262.patch)** — Build on QNX: sighandler_t and the test262 runner.

> - sighandler_t is a glibc name and QNX has neither it nor the BSD sig_t;
>   declare the plain function-pointer type.
> - run-test262 is a conformance-suite driver, not part of the engine, and
>   relies on struct dirent's d_type, which QNX does not have. Skip it.
> - test_conv (an internal unit test) trips a GCC 12 aarch64 false positive
>   under -Werror; it is not built on QNX.

Touches: `quickjs-libc.c`, `CMakeLists.txt` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/quickjs-ng -B build/$QNX_ARCH/quickjs-ng -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DQJS_BUILD_EXAMPLES=OFF
cmake --build build/$QNX_ARCH/quickjs-ng -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/quickjs-ng
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH quickjs-ng`.

### 5. What it installs

17 files under `/usr/local`: 3 shared-library entries, 2 programs, 1 headers.

```
/usr/local/lib/libqjs.so
/usr/local/lib/libqjs.so.0
/usr/local/lib/libqjs.so.0.8.0
/usr/local/bin/qjs
/usr/local/bin/qjsc
```

The complete list is `stage/<arch>/.manifest/quickjs-ng` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH quickjs-ng
ls repo/8.0/*/ | grep '^quickjs-ng-'
```

This writes `quickjs-ng-<ver>-r0.tar.gz` (run time) and `quickjs-ng-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install quickjs-ng
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/quickjs-ng-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/quickjs-ng-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_quickjs`](../../tests/src/) links quickjs-ng the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_quickjs root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_quickjs'
```

```
== quickjs-ng ==
  ok   ...
PASS quickjs-ng
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.8.0; GIT=https://github.com/quickjs-ng/quickjs.git; REF=v0.8.0
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DQJS_BUILD_EXAMPLES=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
