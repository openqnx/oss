# mesa 26.2.6

> Mesa 3D: software OpenGL, GLES, EGL (llvmpipe) and Vulkan (lavapipe)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.mesa3d.org/mesa-26.2.6.tar.xz |
| SHA-256 | `b9436b3e1a8be76fbd4823dc884f90596ab7c6ac5e1867a305e66ffccfb31a92` |
| Build system | Meson |
| QNX patches | 6 |
| Build dependencies | [`llvm`](../llvm/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`zlib`](../zlib/) [`zstd`](../zstd/) [`expat`](../expat/) |
| Run-time dependencies | [`expat`](../expat/) [`llvm`](../llvm/) [`wayland`](../wayland/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_mesa_wayland`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `mesa`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`mesa`](https://openqnx.github.io/oss/package/x86_64/mesa.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/mesa.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add mesa` (dependencies come along); one version: `apk add mesa=<version>` | `apk del mesa` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install mesa` (dependencies first) | `oss-get remove mesa` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-mesa-wayland && /usr/local/bin/qnx-port-tests/test_mesa_wayland    # its on-target test, from tests-8.0-<arch>-m-z
apk del mesa qnx-test-mesa-wayland
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/mesa.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/mesa.list
rm var/lib/manual/mesa.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild mesa` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (19 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild llvm wayland wayland-protocols zlib zstd expat
```

<details><summary>Full build chain (19)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`llvm`](../llvm/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.mesa3d.org/mesa-26.2.6.tar.xz
mkdir -p dist src/mesa
curl -fL -o dist/$(basename $URL) $URL
echo "b9436b3e1a8be76fbd4823dc884f90596ab7c6ac5e1867a305e66ffccfb31a92  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/mesa --strip-components=1
```

### 3. Apply the QNX patches

6 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/mesa < ports/mesa/patches/0001-qnx-dri-without-drm.patch
patch -p1 -d src/mesa < ports/mesa/patches/0002-qnx-util-os.patch
patch -p1 -d src/mesa < ports/mesa/patches/0003-qnx-memstream.patch
patch -p1 -d src/mesa < ports/mesa/patches/0004-qnx-alloca-builtin.patch
patch -p1 -d src/mesa < ports/mesa/patches/0005-qnx-wsi-no-monotonic-raw.patch
patch -p1 -d src/mesa < ports/mesa/patches/0006-egl-wayland-swrast-without-libdrm.patch
```

**[`0001-qnx-dri-without-drm.patch`](patches/0001-qnx-dri-without-drm.patch)** — Build DRI (for EGL on the software rasterizers) on QNX, which has no DRM

> Mesa builds its DRI interface, which is also how EGL reaches the software
> rasterizers, only for systems it lists as having DRM/KMS (Linux, the BSDs,
> Solaris, Android), or with zink. QNX has no DRM kernel interface, so EGL
> was refused ("EGL requires DRI, Haiku, Windows or Android") although
> llvmpipe/softpipe need none. On QNX, with a software gallium driver, build
> DRI as the GNU Hurd does ("pseudo-drm": dri2.c and the swrast screens,
> without libdrm), which gives EGL's surfaceless and wayland platforms on
> llvmpipe/softpipe. libdrm, GBM and kms_swrast stay off (system_has_kms_drm
> is still false).

Touches: `meson.build` · Upstream status: QNX-specific

**[`0002-qnx-util-os.patch`](patches/0002-qnx-util-os.patch)** — Teach Mesa's OS utilities about QNX

> QNX was not among the operating systems Mesa's utilities know:
> - detect_os.h: DETECT_OS_QNX, and DETECT_OS_POSIX for it, so the POSIX
>   paths (dlfcn.h/dladdr, clock_gettime, sysconf, process name) are taken
>   instead of "#error Unsupported OS";
> - u_endian.h: QNX has no <endian.h>; take the byte order from the
>   compiler's __BYTE_ORDER__;
> - os_misc.c: the POSIX headers, and the available memory from
>   sysconf(_SC_AVPHYS_PAGES) (no /proc/meminfo);
> - build_id.c: QNX declares dl_iterate_phdr() in <sys/link.h> (no
>   <link.h>), without the ElfW() macro, and passes the callback a const
>   struct dl_phdr_info *.

Touches: `src/util/build_id.c`, `src/util/detect_os.h`, `src/util/os_misc.c`, `src/util/u_endian.h` · Upstream status: QNX-specific

**[`0003-qnx-memstream.patch`](patches/0003-qnx-memstream.patch)** — u_memstream without open_memstream() on QNX

> QNX's libc has no open_memstream() (nor fmemopen/fopencookie). Mesa's
> u_memstream (shader dumps, driconf, the GLSL printers) uses it. On QNX use
> an anonymous tmpfile() and read it back into the caller's buffer on flush
> and close -- the scheme the Windows path already uses -- keeping the write
> position at the end as open_memstream does.

Touches: `src/util/memstream.c`, `src/util/memstream.h` · Upstream status: QNX-specific

**[`0004-qnx-alloca-builtin.patch`](patches/0004-qnx-alloca-builtin.patch)** — Use __builtin_alloca on QNX

> QNX's <alloca.h> defines alloca(n) as "stack check passes ? __builtin_alloca(n)
> : NULL". Mesa uses alloca's result unchecked (NIR_VLA_FILL memsets it), and
> GCC 12 then fails with -Werror=nonnull on the NULL branch. Use the compiler
> builtin, as on every other system Mesa builds for.

Touches: `include/c99_alloca.h` · Upstream status: QNX-specific

**[`0005-qnx-wsi-no-monotonic-raw.patch`](patches/0005-qnx-wsi-no-monotonic-raw.patch)** — Vulkan Wayland WSI without CLOCK_MONOTONIC_RAW

> QNX has no CLOCK_MONOTONIC_RAW (only CLOCK_REALTIME, CLOCK_MONOTONIC and
> the CPU-time clocks). The Vulkan Wayland WSI maps the compositor's
> presentation clock to Vulkan time domains; leave the MONOTONIC_RAW case out
> where the clock does not exist, as the Intel driver already does.

Touches: `src/vulkan/wsi/wsi_common_wayland.c` · Upstream status: QNX-specific

**[`0006-egl-wayland-swrast-without-libdrm.patch`](patches/0006-egl-wayland-swrast-without-libdrm.patch)** — EGL Wayland: wl_shm path directly when built without libdrm

> Without libdrm (QNX has no DRM kernel interface) EGL's Wayland platform
> can only present through wl_shm, but eglInitialize first tried the DRM
> path, then zink, and only then software: "failed to get driver name for
> fd -1" / "failed to create dri2 screen" warnings on every display. When
> Mesa is built without libdrm, initialise the Wayland display as a
> software one (as LIBGL_ALWAYS_SOFTWARE=1 does) and take the wl_shm path
> directly.

Touches: `src/egl/drivers/dri2/platform_wayland.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/mesa; B=build/$QNX_ARCH/mesa; mkdir -p $B
```

```bash
do_configure() {
  _mesa_py || return 1
  _mesa_llvm_config || return 1
  local cf="$B/cross.ini" nf=()
  # The cross file plus the stand-in llvm-config and the run path (in the
  # cross file's link arguments: a -Dc_link_args would replace them), and
  # libregex: driconf's application matching uses regcomp(), which QNX keeps
  # out of libc.
  sed -e "s#^\[binaries\]#[binaries]\nllvm-config = '$B/llvm-config'#" \
      -e "s#^\(c\(pp\)\?_link_args *= *\[\)#\1'-Wl,-rpath,$OPT/lib:$PREFIX/lib', #" \
      -e "s#'-lfsnotify'\]#'-lfsnotify', '-lregex']#" \
      "$QNX_MESON_DIR/meson-$arch.ini" > "$cf"
  [ -f "$QNX_MESON_DIR/meson-native.ini" ] && nf=( --native-file "$QNX_MESON_DIR/meson-native.ini" )
  PYTHONPATH="$MESA_PY" "$MESA_PY/meson" setup "$B" "$S" --cross-file "$cf" "${nf[@]}" "${MESON_ARGS_EXTRA[@]}"
}
```

```bash
do_build() { PYTHONPATH="$MESA_PY" ninja -C "$B" -j6; }
```

```bash
do_install() {
  PYTHONPATH="$MESA_PY" DESTDIR="$STAGE" "$MESA_PY/meson" install -C "$B" --no-rebuild
  local d="$STAGE$OPT/lib/dropin"; mkdir -p "$d"
  ln -sf ../libEGL_mesa.so.1 "$d/libEGL.so.1"; ln -sf ../libGLESv2_mesa.so.2 "$d/libGLESv2.so.2"
  ln -sf ../libGLESv1_CM_mesa.so.1 "$d/libGLESv1_CM.so.1"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- mesa: the Mesa 3D graphics library as a software renderer -- desktop OpenGL (4.6 core), OpenGL ES 1/2/3.2 and EGL on llvmpipe (LLVM-JIT rasterizer, from the llvm component) and softpipe, plus lavapipe (software Vulkan 1.4). EGL platforms: surfaceless (pbuffers, off-screen rendering) and wayland (wl_shm buffers: any Wayland compositor, labwc included). The SDP's own libEGL/libGLESv2 (for QNX Screen, the GPU drivers) keep /usr/lib: Mesa goes in a prefix of its own, /usr/local/opt/mesa, nothing linked into bin/ or lib/. Its libraries are renamed (libEGL_mesa.so.1, libGLESv2_mesa.so.2, libGLESv1_CM_mesa.so.1): the SDP's are libEGL.so.1 and libGLESv2.so.1, and QNX's loader takes LD_LIBRARY_PATH before RPATH, so one soname in two places would hand the compositor (wlroots, on the SDP's EGL) Mesa's library or a Mesa client the SDP's. Applications link Mesa through its pkg-config files (egl.pc, glesv2.pc in opt/mesa/lib/pkgconfig), or load libEGL_mesa.so.1 by path (SDL: SDL_VIDEO_EGL_DRIVER).
- No GPU drivers and no DRM/KMS: QNX has no DRM kernel interface, so libdrm, GBM, the EGL "drm" platform and kms_swrast are off (Mesa's meson.build makes that choice itself for a system it does not list as having KMS). No GLX/X11 (the X server here is Xwayland; X clients keep using what they use), no glvnd (not ported). Valgrind/libunwind/lmsensors annotations off. Run path: the prefix's lib/ and /usr/local/lib (libLLVM.so.22.1, libz, libzstd, libexpat, libwayland-*), absolute: QNX's loader does not expand $ORIGIN.
- Build-machine Python modules Mesa's generators need, which neither the host nor hosttools/ has: Mako and MarkupSafe (code templates), PyYAML, packaging, and a Meson new enough for Mesa (>= 1.4; the host's is 1.3). All pure Python, pinned by checksum, unpacked into build/host/mesa-python once.
- LLVM: Mesa asks llvm-config, and the staged one is a QNX program. This stand-in answers the questions Meson asks it from the staged LLVM (its version, components, flags, the shared libLLVM).
- !/bin/sh Generated by ports/mesa/recipe.sh: llvm-config for the staged QNX LLVM.
- LLVM-linked C++: moderate parallelism (other builds share the machine).
- lib/dropin/: the usual names (libEGL.so.1, libGLESv2.so.2, libGLESv1_CM.so.1) as links to Mesa's, for programs that dlopen() them (SDL, GLFW, libepoxy): LD_LIBRARY_PATH=/usr/local/opt/mesa/lib/dropin for that one program chooses Mesa. Never on a system-wide path (the compositor needs the SDP's).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH mesa`.

### 5. What it installs

43 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/mesa` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa mesa
ls dists/8.0/*/ | grep '^mesa-'
```

This writes `mesa-<ver>-r0.tar.gz` (run time) and `mesa-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add mesa
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/mesa-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/mesa-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_mesa_wayland`](../../tests/src/) links mesa the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-mesa-wayland`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-mesa-wayland
/usr/local/bin/qnx-port-tests/test_mesa_wayland
```

```
== mesa ==
  ok   ...
PASS mesa
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_mesa_wayland` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_mesa_wayland`](../../tests/src/) -- a program linked against mesa the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - runtime directory
   - compositor socket
   - compositor offers wl_shm
   - client connects
   - client binds wl_compositor
   - EGL client extensions list EGL_EXT_platform_wayland
   - EGL on the Wayland display initialises
   - window config
   - window surface current
   - renderer is llvmpipe
   - eglSwapBuffers (frame 1)
   - compositor received frame 1
   - buffer is 64x64
   - buffer is (A|X)RGB8888
   - 1024 red pixels
   - red is the bottom-left quarter (rows 32..63, columns 0..31), the rest blue
   - eglSwapBuffers (frame 2)
   - compositor received frame 2
   - frame 2 is all green

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | mesa 26.2.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/mesa-26.2.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | mesa 26.2.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/mesa-26.2.6-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **26.2.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add mesa=<version>`), sources in `ported/mesa/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates mesa`; port one with `scripts/port-update mesa`.

<details><summary>recipe.sh</summary>

```bash
# mesa: the Mesa 3D graphics library as a software renderer -- desktop OpenGL
# (4.6 core), OpenGL ES 1/2/3.2 and EGL on llvmpipe (LLVM-JIT rasterizer,
# from the llvm component) and softpipe, plus lavapipe (software Vulkan 1.4).
# EGL platforms: surfaceless (pbuffers, off-screen rendering) and wayland
# (wl_shm buffers: any Wayland compositor, labwc included). The SDP's own
# libEGL/libGLESv2 (for QNX Screen, the GPU drivers) keep /usr/lib: Mesa goes
# in a prefix of its own, /usr/local/opt/mesa, nothing linked into bin/ or
# lib/. Its libraries are renamed (libEGL_mesa.so.1, libGLESv2_mesa.so.2,
# libGLESv1_CM_mesa.so.1): the SDP's are libEGL.so.1 and libGLESv2.so.1, and
# QNX's loader takes LD_LIBRARY_PATH before RPATH, so one soname in two
# places would hand the compositor (wlroots, on the SDP's EGL) Mesa's
# library or a Mesa client the SDP's. Applications link Mesa through its
# pkg-config files (egl.pc, glesv2.pc in opt/mesa/lib/pkgconfig), or load
# libEGL_mesa.so.1 by path (SDL: SDL_VIDEO_EGL_DRIVER).
VERSION=26.2.6
TARBALL=https://archive.mesa3d.org/mesa-26.2.6.tar.xz
SHA256=b9436b3e1a8be76fbd4823dc884f90596ab7c6ac5e1867a305e66ffccfb31a92
DEPS="llvm wayland wayland-protocols zlib zstd expat"
BUILDSYS=meson
OPT="$PREFIX/opt/mesa"
# No GPU drivers and no DRM/KMS: QNX has no DRM kernel interface, so libdrm,
# GBM, the EGL "drm" platform and kms_swrast are off (Mesa's meson.build makes
# that choice itself for a system it does not list as having KMS). No GLX/X11
# (the X server here is Xwayland; X clients keep using what they use), no
# glvnd (not ported). Valgrind/libunwind/lmsensors annotations off.
# Run path: the prefix's lib/ and /usr/local/lib (libLLVM.so.22.1, libz,
# libzstd, libexpat, libwayland-*), absolute: QNX's loader does not expand
# $ORIGIN.
MESA_PLATFORMS=wayland
MESON_ARGS_EXTRA=( --prefix="$OPT" -Dlibdir=lib -Dbuildtype=release -Db_ndebug=true
  -Dplatforms=$MESA_PLATFORMS -Degl-native-platform=surfaceless
  -Dgallium-drivers=llvmpipe,softpipe -Dvulkan-drivers=swrast
  -Degl-lib-suffix=_mesa -Dgles-lib-suffix=_mesa
  -Degl=enabled -Dgles1=enabled -Dgles2=enabled -Dopengl=true -Dglx=disabled -Dgbm=disabled -Dglvnd=disabled
  -Dllvm=enabled -Dshared-llvm=enabled -Ddraw-use-llvm=true
  -Dgallium-va=disabled -Dgallium-rusticl=false -Dteflon=false
  -Dvalgrind=disabled -Dlibunwind=disabled -Dlmsensors=disabled -Dxlib-lease=disabled
  -Dzstd=enabled -Dxmlconfig=enabled -Dexpat=enabled -Dshader-cache=enabled
  -Dbuild-tests=false -Dtools=[] -Dhtml-docs=disabled -Dvulkan-layers=[]
  -Dcpp_rtti=true )

# Build-machine Python modules Mesa's generators need, which neither the host
# nor hosttools/ has: Mako and MarkupSafe (code templates), PyYAML, packaging,
# and a Meson new enough for Mesa (>= 1.4; the host's is 1.3). All pure
# Python, pinned by checksum, unpacked into build/host/mesa-python once.
MESA_PY="$BUILD_DIR/host/mesa-python"
_mesa_py() {
  [ -f "$MESA_PY/.done" ] && return 0
  rm -rf "$MESA_PY"; mkdir -p "$MESA_PY/src" "$WS/dist"
  local e u f sum
  for e in \
    "https://files.pythonhosted.org/packages/87/fb/99f81ac72ae23375f22b7afdb7642aba97c00a713c217124420147681a2f/mako-1.3.10-py3-none-any.whl baef24a52fc4fc514a0887ac600f9f1cff3d82c61d4d700a1fa84d597b88db59" \
    "https://files.pythonhosted.org/packages/7e/99/7690b6d4034fffd95959cbe0c02de8deb3098cc577c67bb6a24fe5d7caa7/markupsafe-3.0.3.tar.gz 722695808f4b6457b320fdc131280796bdceb04ab50fe1795cd540799ebe1698" \
    "https://files.pythonhosted.org/packages/05/8e/961c0007c59b8dd7729d542c61a4d537767a59645b82a0b521206e1e25c2/pyyaml-6.0.3.tar.gz d76623373421df22fb4cf8817020cbb7ef15c725b9d5e45f17e189bfc384190f" \
    "https://files.pythonhosted.org/packages/20/12/38679034af332785aac8774540895e234f4d07f7545804097de4b666afd8/packaging-25.0-py3-none-any.whl 29572ef2b1f17581046b3a2227d5c611fb25ec70ca1ba8554b24b0e69331a484" \
    "https://files.pythonhosted.org/packages/32/4f/c398c6f06ece1c6c246e008d5dac3824c98f54d3eb3d8014f4910afd6d48/meson-1.10.0-py3-none-any.whl 4b27aafce281e652dcb437b28007457411245d975c48b5db3a797d3e93ae1585"; do
    u=${e% *}; sum=${e#* }; f="$WS/dist/${u##*/}"
    [ -f "$f" ] || curl -fsSL --retry 3 -o "$f" "$u" || return 1
    echo "$sum  $f" | sha256sum -c --status || { echo "!! checksum mismatch: $f"; rm -f "$f"; return 1; }
    case "$f" in
      *.whl) python3 -m zipfile -e "$f" "$MESA_PY" || return 1 ;;
      *) tar xf "$f" -C "$MESA_PY/src" || return 1 ;;
    esac
  done
  # The two source archives: their pure-Python packages, without the C
  # speedups (both fall back to Python when those are absent).
  cp -r "$MESA_PY"/src/markupsafe-*/src/markupsafe "$MESA_PY"/src/pyyaml-*/lib/yaml "$MESA_PY/" || return 1
  # A real script (Meson writes its own path into build.ninja), which finds
  # these modules next to itself.
  printf '#!/usr/bin/env python3\nimport os, sys\nsys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))\nfrom mesonbuild.mesonmain import main\nsys.exit(main())\n' > "$MESA_PY/meson"
  chmod 755 "$MESA_PY/meson"; touch "$MESA_PY/.done"
}

# LLVM: Mesa asks llvm-config, and the staged one is a QNX program. This
# stand-in answers the questions Meson asks it from the staged LLVM (its
# version, components, flags, the shared libLLVM).
_mesa_llvm_config() {
  local L="$STAGE$PREFIX" v comps
  v=$(sed -n 's/^set(LLVM_PACKAGE_VERSION \(.*\))/\1/p' "$L/lib/cmake/llvm/LLVMConfigVersion.cmake" | head -1)
  [ -n "$v" ] || v=$(sed -n 's/.*LLVM_VERSION_STRING "\(.*\)".*/\1/p' "$L/include/llvm/Config/llvm-config.h")
  comps=$(cd "$L/lib" && ls libLLVM*.a | sed 's/^libLLVM\(.*\)\.a$/\1/' | tr 'A-Z' 'a-z' | tr '\n' ' ')
  comps="$comps all all-targets native nativecodegen engine aarch64 x86 webassembly"
  cat > "$B/llvm-config" <<EOF
#!/bin/sh
# Generated by ports/mesa/recipe.sh: llvm-config for the staged QNX LLVM.
out=""
for a in "\$@"; do case "\$a" in
  --version) out="\$out $v" ;;
  --components) out="\$out $comps" ;;
  --shared-mode) out="\$out shared" ;;
  --has-rtti) out="\$out YES" ;;
  --libdir) out="\$out $L/lib" ;;
  --includedir) out="\$out $L/include" ;;
  --prefix) out="\$out $L" ;;
  --bindir) out="\$out $L/bin" ;;
  --cppflags) out="\$out -I$L/include -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS" ;;
  --cxxflags) out="\$out -I$L/include -std=c++17 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS" ;;
  --cflags) out="\$out -I$L/include -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS" ;;
  --ldflags) out="\$out -L$L/lib" ;;
  --libs) out="\$out -lLLVM" ;;
  --libfiles) out="\$out $L/lib/libLLVM.so" ;;
  --system-libs|--link-shared|--link-static) : ;;
  --targets-built) out="\$out AArch64 X86 WebAssembly" ;;
  --host-target) out="\$out $([ "$QNX_CPU" = aarch64 ] && echo aarch64-unknown-qnx || echo x86_64-pc-qnx)" ;;
  --assertion-mode) out="\$out OFF" ;;
  --build-mode) out="\$out Release" ;;
  -*) echo "llvm-config: unsupported \$a" >&2; exit 1 ;;
  *) : ;;
esac; done
echo \$out
EOF
  chmod 755 "$B/llvm-config"
}

do_configure() {
  _mesa_py || return 1
  _mesa_llvm_config || return 1
  local cf="$B/cross.ini" nf=()
  # The cross file plus the stand-in llvm-config and the run path (in the
  # cross file's link arguments: a -Dc_link_args would replace them), and
  # libregex: driconf's application matching uses regcomp(), which QNX keeps
  # out of libc.
  sed -e "s#^\[binaries\]#[binaries]\nllvm-config = '$B/llvm-config'#" \
      -e "s#^\(c\(pp\)\?_link_args *= *\[\)#\1'-Wl,-rpath,$OPT/lib:$PREFIX/lib', #" \
      -e "s#'-lfsnotify'\]#'-lfsnotify', '-lregex']#" \
      "$QNX_MESON_DIR/meson-$arch.ini" > "$cf"
  [ -f "$QNX_MESON_DIR/meson-native.ini" ] && nf=( --native-file "$QNX_MESON_DIR/meson-native.ini" )
  PYTHONPATH="$MESA_PY" "$MESA_PY/meson" setup "$B" "$S" --cross-file "$cf" "${nf[@]}" "${MESON_ARGS_EXTRA[@]}"
}
# LLVM-linked C++: moderate parallelism (other builds share the machine).
do_build() { PYTHONPATH="$MESA_PY" ninja -C "$B" -j6; }
# lib/dropin/: the usual names (libEGL.so.1, libGLESv2.so.2, libGLESv1_CM.so.1)
# as links to Mesa's, for programs that dlopen() them (SDL, GLFW, libepoxy):
# LD_LIBRARY_PATH=/usr/local/opt/mesa/lib/dropin for that one program
# chooses Mesa. Never on a system-wide path (the compositor needs the SDP's).
do_install() {
  PYTHONPATH="$MESA_PY" DESTDIR="$STAGE" "$MESA_PY/meson" install -C "$B" --no-rebuild
  local d="$STAGE$OPT/lib/dropin"; mkdir -p "$d"
  ln -sf ../libEGL_mesa.so.1 "$d/libEGL.so.1"; ln -sf ../libGLESv2_mesa.so.2 "$d/libGLESv2.so.2"
  ln -sf ../libGLESv1_CM_mesa.so.1 "$d/libGLESv1_CM.so.1"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
