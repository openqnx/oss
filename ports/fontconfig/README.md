# fontconfig 2.15.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/fontconfig/fontconfig.git (tag `2.15.0`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`freetype`](../freetype/) [`expat`](../expat/) |
| Run-time dependencies | [`expat`](../expat/) [`freetype`](../freetype/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_fontconfig`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install fontconfig
oss-get install fontconfig-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild fontconfig` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (6 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild freetype expat
```

<details><summary>Full build chain (6)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`expat`](../expat/) [`freetype`](../freetype/) [`libpng`](../libpng/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 2.15.0 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/fontconfig/fontconfig.git src/fontconfig
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/fontconfig src/fontconfig \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Ddoc=disabled \
      -Dtests=disabled \
      -Dtools=disabled \
      -Dnls=enabled \
      -Dcache-build=disabled \
      -Diconv=disabled
ninja -C build/$QNX_ARCH/fontconfig
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/fontconfig install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH fontconfig`.

### 5. What it installs

79 files under `/usr/local`: 3 shared-library entries, 3 headers.

```
/usr/local/lib/libfontconfig.so
/usr/local/lib/libfontconfig.so.1
/usr/local/lib/libfontconfig.so.1.14.0
```

The complete list is `stage/<arch>/.manifest/fontconfig` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH fontconfig
ls repo/8.0/*/ | grep '^fontconfig-'
```

This writes `fontconfig-<ver>-r0.tar.gz` (run time) and `fontconfig-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install fontconfig
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/fontconfig-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/fontconfig-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_fontconfig`](../../tests/src/) links fontconfig the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_fontconfig root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_fontconfig'
```

```
== fontconfig ==
  ok   ...
PASS fontconfig
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.15.0; GIT=https://gitlab.freedesktop.org/fontconfig/fontconfig.git; REF=2.15.0
DEPS="freetype expat"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Ddoc=disabled -Dtests=disabled -Dtools=disabled
                   -Dnls=enabled -Dcache-build=disabled -Diconv=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
