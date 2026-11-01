# virtualgl 3.1.5

> VirtualGL client (vglclient) for displaying remotely rendered OpenGL

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/VirtualGL/virtualgl.git (tag `3.1.5`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | [`libx11`](../libx11/) [`libxext`](../libxext/) [`libjpeg-turbo`](../libjpeg-turbo/) |
| Run-time dependencies | [`libjpeg-turbo`](../libjpeg-turbo/) [`libx11`](../libx11/) [`libxext`](../libxext/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_virtualgl`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `virtualgl` (programs, shared libraries, run-time data) and `virtualgl-dev` (headers, pkg-config/CMake files, static libraries; needs `virtualgl`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`virtualgl`](https://openqnx.github.io/oss/package/x86_64/virtualgl.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/virtualgl.html)) · [`virtualgl-dev`](https://openqnx.github.io/oss/package/x86_64/virtualgl-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/virtualgl-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add virtualgl` (dependencies come along); one version: `apk add virtualgl=<version>` | `apk del virtualgl` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install virtualgl` (dependencies first) | `oss-get remove virtualgl` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`virtualgl-dev` the same way, by its own name (`apk add virtualgl-dev`, `oss-get install virtualgl-dev`, by hand with its own file and list `virtualgl-dev.list`); remove it before `virtualgl`, which it needs.

```sh
# on the QNX target, as root
apk add virtualgl virtualgl-dev
apk add qnx-test-virtualgl && /usr/local/bin/qnx-port-tests/test_virtualgl    # its on-target test, from tests-8.0-<arch>-m-z
apk del virtualgl virtualgl-dev qnx-test-virtualgl
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/virtualgl.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/virtualgl.list
rm var/lib/manual/virtualgl.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild virtualgl` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (10 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libx11 libxext libjpeg-turbo
```

<details><summary>Full build chain (10)</summary>

[`libjpeg-turbo`](../libjpeg-turbo/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`pthread-stubs`](../pthread-stubs/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.1.5 --recurse-submodules --shallow-submodules \
    https://github.com/VirtualGL/virtualgl.git src/virtualgl
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/virtualgl < ports/virtualgl/patches/0001-client-without-opengl.patch
patch -p1 -d src/virtualgl < ports/virtualgl/patches/0002-qnx-no-sysv-shm.patch
```

**[`0001-client-without-opengl.patch`](patches/0001-client-without-opengl.patch)** — Build the client without desktop OpenGL (X11 drawing only)

> vglclient draws the frames it receives with X11 (XShm/XPutImage); OpenGL
> (GLX) is used only for stereo frames, through GLFrame -- and when GLX fails
> at run time, ClientWin already falls back to X11 drawing and turns stereo
> off. QNX has no desktop OpenGL (EGL and OpenGL ES only), so the client could
> not be built at all. A VGL_USEGL option (default on, unchanged): off, GLFrame
> is built without GLX and its constructors throw, which takes that same
> fallback; the OpenGL programs (glreadtest, the demos, the diagnostics) are
> not built.

Touches: `client/GLFrame.cpp`, `client/GLFrame.h`, `CMakeLists.txt`, `util/CMakeLists.txt` · Upstream status: Pending

**[`0002-qnx-no-sysv-shm.patch`](patches/0002-qnx-no-sysv-shm.patch)** — Draw without MIT-SHM on QNX (no System V shared memory)

> fbx draws with MIT-SHM (XShmPutImage) when USESHM is defined, and with
> XPutImage otherwise; USESHM is defined unconditionally. MIT-SHM needs System
> V shared memory (<sys/shm.h>, shmget/shmat), which QNX does not have. On QNX
> leave it undefined: frames go to the X server with XPutImage.

Touches: `include/fbx.h` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/virtualgl -B build/$QNX_ARCH/virtualgl -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DVGL_BUILDSERVER=OFF \
      -DVGL_USEGL=OFF \
      -DVGL_USEXV=OFF \
      -DVGL_FAKEXCB=OFF \
      -DTJPEG_INCLUDE_DIR="$STAGE$PREFIX/include" \
      -DTJPEG_LIBRARY="$STAGE$PREFIX/lib/libturbojpeg.so" \
      -DCMAKE_INSTALL_DOCDIR=share/doc/virtualgl \
      -DCMAKE_INSTALL_BINDIR=bin
cmake --build build/$QNX_ARCH/virtualgl -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/virtualgl
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- virtualgl: the client side of VirtualGL -- vglclient, which receives the OpenGL frames a VirtualGL server (vglrun on a Linux machine with a GPU, e.g. RViz in an Ubuntu guest) renders and draws them on this machine's X display (Xwayland on the QNX desktop), and vglconnect, which sets up that link over SSH. The server side (the GL faker, vglrun) belongs on the Linux machine and is not built: it needs desktop OpenGL 4.5, which QNX does not have.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH virtualgl`.

### 5. What it installs

16 files under `/usr/local`, 3 programs.

```
/usr/local/bin/nettest
/usr/local/bin/vglclient
/usr/local/bin/vglconnect
```

The complete list is `stage/<arch>/.manifest/virtualgl` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa virtualgl
ls dists/8.0/*/ | grep '^virtualgl-'
```

This writes `virtualgl-<ver>-r0.tar.gz` (run time) and `virtualgl-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add virtualgl
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/virtualgl-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/virtualgl-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_virtualgl`](../../tests/src/) links virtualgl the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-virtualgl`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-virtualgl
/usr/local/bin/qnx-port-tests/test_virtualgl
```

```
== virtualgl ==
  ok   ...
PASS virtualgl
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_virtualgl` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_virtualgl`](../../tests/src/) -- a program linked against virtualgl the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - labwc up (Wayland compositor)
   - Xwayland up (an X display on labwc)
   - XOpenDisplay
   - X11: drawn colour read back exactly
   - vglclient listening (VGL transport)
   - version handshake (VGL 2.x)
   - frame sent: JPEG tile + end of frame
   - vglclient drew the frame in the window

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xwayland 24.1.14-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/xwayland-24.1.14-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | virtualgl 3.1.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/virtualgl-3.1.5-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xwayland 24.1.14-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/xwayland-24.1.14-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xwayland 24.1.14-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/xwayland-24.1.14-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | virtualgl 3.1.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/virtualgl-3.1.5-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xwayland 24.1.14-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/xwayland-24.1.14-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.1.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add virtualgl=<version>`), sources in `ported/virtualgl/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates virtualgl`; port one with `scripts/port-update virtualgl`.

<details><summary>recipe.sh</summary>

```bash
# virtualgl: the client side of VirtualGL -- vglclient, which receives the
# OpenGL frames a VirtualGL server (vglrun on a Linux machine with a GPU, e.g.
# RViz in an Ubuntu guest) renders and draws them on this machine's X display
# (Xwayland on the QNX desktop), and vglconnect, which sets up that link over
# SSH. The server side (the GL faker, vglrun) belongs on the Linux machine and
# is not built: it needs desktop OpenGL 4.5, which QNX does not have.
VERSION=3.1.5; GIT=https://github.com/VirtualGL/virtualgl.git; REF=3.1.5
DEPS="libx11 libxext libjpeg-turbo"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DVGL_BUILDSERVER=OFF -DVGL_USEGL=OFF -DVGL_USEXV=OFF -DVGL_FAKEXCB=OFF -DTJPEG_INCLUDE_DIR="$STAGE$PREFIX/include"
                   -DTJPEG_LIBRARY="$STAGE$PREFIX/lib/libturbojpeg.so" -DCMAKE_INSTALL_DOCDIR=share/doc/virtualgl
                   -DCMAKE_INSTALL_BINDIR=bin )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
