# libepoxy 1.5.10

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/anholt/libepoxy.git (tag `1.5.10`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`libx11`](../libx11/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libepoxy`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libepoxy
oss-get install libepoxy-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libepoxy` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (8 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libx11
```

<details><summary>Full build chain (8)</summary>

[`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`pthread-stubs`](../pthread-stubs/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.5.10 --recurse-submodules --shallow-submodules \
    https://github.com/anholt/libepoxy.git src/libepoxy
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libepoxy src/libepoxy \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dglx=yes \
      -Degl=yes \
      -Dx11=true \
      -Dtests=false \
      -Ddocs=false
ninja -C build/$QNX_ARCH/libepoxy
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libepoxy install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- EGL dispatch for QNX's Screen EGL, plus GLX dispatch.  GLX was off at first ("QNX has no X11"), but GTK3's X11 backend -- needed by Thunar -- includes <epoxy/glx.h> unconditionally.  Building it needs only the X11 headers: epoxy dlopens libGL on first use, and GTK asks through epoxy_has_glx(), which loads with exit_on_fail=false and simply answers false when there is no libGL (dispatch_glx.c).  No GL implementation is built or linked here.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libepoxy`.

### 5. What it installs

12 files under `/usr/local`: 3 shared-library entries, 7 headers.

```
/usr/local/lib/libepoxy.so
/usr/local/lib/libepoxy.so.0
/usr/local/lib/libepoxy.so.0.0.0
```

The complete list is `stage/<arch>/.manifest/libepoxy` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libepoxy
ls repo/8.0/*/ | grep '^libepoxy-'
```

This writes `libepoxy-<ver>-r0.tar.gz` (run time) and `libepoxy-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libepoxy
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libepoxy-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libepoxy-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libepoxy`](../../tests/src/) links libepoxy the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libepoxy root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libepoxy'
```

```
== libepoxy ==
  ok   ...
PASS libepoxy
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.5.10; GIT=https://github.com/anholt/libepoxy.git; REF=1.5.10
DEPS="libx11"
BUILDSYS=meson
# EGL dispatch for QNX's Screen EGL, plus GLX dispatch.  GLX was off at first
# ("QNX has no X11"), but GTK3's X11 backend -- needed by Thunar -- includes
# <epoxy/glx.h> unconditionally.  Building it needs only the X11 headers:
# epoxy dlopens libGL on first use, and GTK asks through epoxy_has_glx(), which
# loads with exit_on_fail=false and simply answers false when there is no libGL
# (dispatch_glx.c).  No GL implementation is built or linked here.
MESON_ARGS_EXTRA=( -Dglx=yes -Degl=yes -Dx11=true -Dtests=false -Ddocs=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
