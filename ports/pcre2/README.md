# pcre2 10.44

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/PCRE2Project/pcre2.git (tag `pcre2-10.44`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) [`bzip2`](../bzip2/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_pcre2`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install pcre2
oss-get install pcre2-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pcre2` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild zlib bzip2
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b pcre2-10.44 --recurse-submodules --shallow-submodules \
    https://github.com/PCRE2Project/pcre2.git src/pcre2
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/pcre2 -B build/$QNX_ARCH/pcre2 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DPCRE2_BUILD_PCRE2_8=ON \
      -DPCRE2_BUILD_PCRE2_16=ON \
      -DPCRE2_BUILD_PCRE2_32=ON \
      -DPCRE2_BUILD_TESTS=OFF \
      -DPCRE2_BUILD_PCRE2GREP=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/pcre2 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/pcre2
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pcre2`.

### 5. What it installs

220 files under `/usr/local`: 12 shared-library entries, 1 programs, 2 headers.

```
/usr/local/lib/libpcre2-16.so
/usr/local/lib/libpcre2-16.so.0
/usr/local/lib/libpcre2-16.so.0.13.0
/usr/local/lib/libpcre2-32.so
/usr/local/lib/libpcre2-32.so.0
/usr/local/lib/libpcre2-32.so.0.13.0
/usr/local/lib/libpcre2-8.so
/usr/local/lib/libpcre2-8.so.0
/usr/local/lib/libpcre2-8.so.0.13.0
/usr/local/lib/libpcre2-posix.so
/usr/local/lib/libpcre2-posix.so.3
/usr/local/lib/libpcre2-posix.so.3.0.5
/usr/local/bin/pcre2-config
```

The complete list is `stage/<arch>/.manifest/pcre2` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH pcre2
ls repo/8.0/*/ | grep '^pcre2-'
```

This writes `pcre2-<ver>-r0.tar.gz` (run time) and `pcre2-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install pcre2
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/pcre2-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/pcre2-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_pcre2`](../../tests/src/) links pcre2 the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_pcre2 root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_pcre2'
```

```
== pcre2 ==
  ok   ...
PASS pcre2
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=10.44; GIT=https://github.com/PCRE2Project/pcre2.git; REF=pcre2-10.44
DEPS="zlib bzip2"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DPCRE2_BUILD_PCRE2_8=ON
                   -DPCRE2_BUILD_PCRE2_16=ON -DPCRE2_BUILD_PCRE2_32=ON
                   -DPCRE2_BUILD_TESTS=OFF -DPCRE2_BUILD_PCRE2GREP=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
