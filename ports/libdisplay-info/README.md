# libdisplay-info 0.3.0

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/emersion/libdisplay-info.git (tag `0.3.0`) |
| Build system | Meson |
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
oss-get install libdisplay-info
oss-get install libdisplay-info-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libdisplay-info` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 0.3.0 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/emersion/libdisplay-info.git src/libdisplay-info
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libdisplay-info < ports/libdisplay-info/patches/0001-qnx-no-open-memstream.patch
```

**[`0001-qnx-no-open-memstream.patch`](patches/0001-qnx-no-open-memstream.patch)** — Work without open_memstream() (QNX).

> QNX 8's C library has no open_memstream(). The library's memory_stream
> helper writes to a tmpfile() on QNX and reads the text back into a string on
> close; di-edid-decode's join_str() builds its string directly.
>
> (QNX's own package includes <sys/memstream.h> from a QNX add-on library that
> is not part of the SDP; this keeps the port buildable with the SDP alone.)

Touches: `memory-stream.c`, `di-edid-decode/edid.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libdisplay-info src/libdisplay-info \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both
ninja -C build/$QNX_ARCH/libdisplay-info
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libdisplay-info install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libdisplay-info: EDID/DisplayID parsing. Version as in the QNX Everywhere image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libdisplay-info`.

### 5. What it installs

14 files under `/usr/local`: 3 shared-library entries, 1 programs, 8 headers.

```
/usr/local/lib/libdisplay-info.so
/usr/local/lib/libdisplay-info.so.0.3.0
/usr/local/lib/libdisplay-info.so.3
/usr/local/bin/di-edid-decode
```

The complete list is `stage/<arch>/.manifest/libdisplay-info` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libdisplay-info
ls repo/8.0/*/ | grep '^libdisplay-info-'
```

This writes `libdisplay-info-<ver>-r0.tar.gz` (run time) and `libdisplay-info-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libdisplay-info
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libdisplay-info-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libdisplay-info-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# libdisplay-info: EDID/DisplayID parsing. Version as in the QNX Everywhere image.
VERSION=0.3.0; GIT=https://gitlab.freedesktop.org/emersion/libdisplay-info.git; REF=0.3.0
BUILDSYS=meson
MESON_ARGS_EXTRA=( )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
