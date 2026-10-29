# libpsl 0.21.5

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/rockdaboot/libpsl.git (tag `0.21.5`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`libidn2`](../libidn2/) [`libunistring`](../libunistring/) [`libiconv`](../libiconv/) |
| Run-time dependencies | [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libunistring`](../libunistring/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libpsl`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libpsl
oss-get install libpsl-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libpsl` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (5 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libidn2 libunistring libiconv
```

<details><summary>Full build chain (5)</summary>

[`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 0.21.5 --recurse-submodules --shallow-submodules \
    https://github.com/rockdaboot/libpsl.git src/libpsl
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libpsl src/libpsl \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Druntime=libidn2 \
      -Dbuiltin=true \
      -Dtests=false \
      -Ddocs=false
ninja -C build/$QNX_ARCH/libpsl
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libpsl install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): libiconv

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libpsl`.

### 5. What it installs

10 files under `/usr/local`: 3 shared-library entries, 2 programs, 1 headers.

```
/usr/local/lib/libpsl.so
/usr/local/lib/libpsl.so.5
/usr/local/lib/libpsl.so.5.3.5
/usr/local/bin/psl
/usr/local/bin/psl-make-dafsa
```

The complete list is `stage/<arch>/.manifest/libpsl` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libpsl
ls repo/8.0/*/ | grep '^libpsl-'
```

This writes `libpsl-<ver>-r0.tar.gz` (run time) and `libpsl-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libpsl
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libpsl-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libpsl-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libpsl`](../../tests/src/) links libpsl the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libpsl root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libpsl'
```

```
== libpsl ==
  ok   ...
PASS libpsl
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.21.5; GIT=https://github.com/rockdaboot/libpsl.git; REF=0.21.5
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="libidn2 libunistring libiconv"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Druntime=libidn2 -Dbuiltin=true -Dtests=false -Ddocs=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
