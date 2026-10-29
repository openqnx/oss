# thorvg 0.15.8

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/thorvg/thorvg.git (tag `v0.15.8`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`zlib`](../zlib/) |
| Run-time dependencies | [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_thorvg`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install thorvg
oss-get install thorvg-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild thorvg` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (3 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libjpeg-turbo libpng zlib
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.15.8 --recurse-submodules --shallow-submodules \
    https://github.com/thorvg/thorvg.git src/thorvg
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/thorvg src/thorvg \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      --default-library=shared \
      -Dstatic=false \
      -Dloaders=svg,png,jpg,lottie \
      -Dsavers=gif \
      -Dtests=false \
      -Dexamples=false \
      -Dbindings=capi \
      -Dlog=false \
      -Dthreads=false
ninja -C build/$QNX_ARCH/thorvg
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/thorvg install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- --default-library=both (the driver's default) makes ThorVG define TVG_STATIC, which turns TVG_API into nothing at all -- the shared library then exports no tvg_* symbol and every consumer fails to link. Build shared only.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH thorvg`.

### 5. What it installs

7 files under `/usr/local`: 3 shared-library entries, 3 headers.

```
/usr/local/lib/libthorvg.so
/usr/local/lib/libthorvg.so.0
/usr/local/lib/libthorvg.so.0.15.8
```

The complete list is `stage/<arch>/.manifest/thorvg` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH thorvg
ls repo/8.0/*/ | grep '^thorvg-'
```

This writes `thorvg-<ver>-r0.tar.gz` (run time) and `thorvg-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install thorvg
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/thorvg-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/thorvg-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_thorvg`](../../tests/src/) links thorvg the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_thorvg root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_thorvg'
```

```
== thorvg ==
  ok   ...
PASS thorvg
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.15.8; GIT=https://github.com/thorvg/thorvg.git; REF=v0.15.8
DEPS="libjpeg-turbo libpng zlib"
BUILDSYS=meson
# --default-library=both (the driver's default) makes ThorVG define TVG_STATIC,
# which turns TVG_API into nothing at all -- the shared library then exports no
# tvg_* symbol and every consumer fails to link. Build shared only.
MESON_ARGS_EXTRA=( --default-library=shared
                   -Dstatic=false
                   -Dloaders=svg,png,jpg,lottie -Dsavers=gif -Dtests=false
                   -Dexamples=false -Dbindings=capi -Dlog=false -Dthreads=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
