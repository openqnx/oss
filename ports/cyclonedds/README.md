# cyclonedds 0.10.5

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/eclipse-cyclonedds/cyclonedds.git (tag `0.10.5`) |
| Build system | CMake |
| QNX patches | 3 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Built alone (empty stage) | aarch64le ✅ x86_64 ✅ |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_cyclonedds`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install cyclonedds
oss-get install cyclonedds-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild cyclonedds` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 0.10.5 --recurse-submodules --shallow-submodules \
    https://github.com/eclipse-cyclonedds/cyclonedds.git src/cyclonedds
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/cyclonedds < ports/cyclonedds/patches/0001-qnx-xlocale-shim.patch
patch -p1 -d src/cyclonedds < ports/cyclonedds/patches/0002-qnx-no-token-fddi-media.patch
patch -p1 -d src/cyclonedds < ports/cyclonedds/patches/0003-qnx-qcc-is-gnuc.patch
```

**[`0001-qnx-xlocale-shim.patch`](patches/0001-qnx-xlocale-shim.patch)** — Cyclone DDS IDL compiler: no xlocale on QNX.

> QNX 8.0 has no locale_t, newlocale/uselocale or the *_l functions, and only
> the C locale. The IDL front end parses ASCII only, so map the *_l calls to
> the plain ones and make the locale handle an inert value.

Touches: `src/idl/src/string.c` · Upstream status: QNX-specific

**[`0002-qnx-no-token-fddi-media.patch`](patches/0002-qnx-no-token-fddi-media.patch)** — Cyclone DDS: skip Token Ring and FDDI interface types on QNX.

> QNX defines IFM_ETHER and IFM_IEEE80211 but not the legacy Token Ring and
> FDDI media types, which ifaddrs.c otherwise uses on every non-FreeBSD BSD-
> style system.

Touches: `src/ddsrt/src/ifaddrs/posix/ifaddrs.c` · Upstream status: QNX-specific

**[`0003-qnx-qcc-is-gnuc.patch`](patches/0003-qnx-qcc-is-gnuc.patch)** — Cyclone DDS idlpp: treat qcc as GCC.

> CMake identifies qcc as "QCC", not "GNU", so HOST_COMPILER stayed empty and
> mcpp's "#if HOST_COMPILER == ..." lines became syntax errors. qcc is a GCC
> driver; take the GNUC branch.

Touches: `src/tools/idlpp/CMakeLists.txt` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/cyclonedds -B build/$QNX_ARCH/cyclonedds -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TESTING=OFF \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_IDLC=ON \
      -DENABLE_SSL=OFF \
      -DENABLE_SECURITY=OFF \
      -DENABLE_LTO=OFF \
      -DENABLE_TOPIC_DISCOVERY=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/cyclonedds -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/cyclonedds
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH cyclonedds`.

### 5. What it installs

214 files under `/usr/local`: 6 shared-library entries, 1 programs, 195 headers.

```
/usr/local/lib/libcycloneddsidl.so
/usr/local/lib/libcycloneddsidl.so.0
/usr/local/lib/libcycloneddsidl.so.0.10.5
/usr/local/lib/libddsc.so
/usr/local/lib/libddsc.so.0
/usr/local/lib/libddsc.so.0.10.5
/usr/local/bin/idlc
```

The complete list is `stage/<arch>/.manifest/cyclonedds` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH cyclonedds
ls repo/8.0/*/ | grep '^cyclonedds-'
```

This writes `cyclonedds-<ver>-r0.tar.gz` (run time) and `cyclonedds-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install cyclonedds
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/cyclonedds-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/cyclonedds-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_cyclonedds`](../../tests/src/) links cyclonedds the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_cyclonedds root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_cyclonedds'
```

```
== cyclonedds ==
  ok   ...
PASS cyclonedds
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.10.5; GIT=https://github.com/eclipse-cyclonedds/cyclonedds.git; REF=0.10.5
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DBUILD_EXAMPLES=OFF
                   -DBUILD_IDLC=ON -DENABLE_SSL=OFF -DENABLE_SECURITY=OFF
                   -DENABLE_LTO=OFF -DENABLE_TOPIC_DISCOVERY=ON
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
