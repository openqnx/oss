# webkitgtk 2.44.4

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://webkitgtk.org/releases/webkitgtk-2.44.4.tar.xz |
| SHA-256 | `2ce4ec1b78413035037aba8326b31ed72696626b7bea7bace5e46ac0d8cbe796` |
| Build system | CMake |
| QNX patches | 5 |
| Build dependencies | [`gtk3`](../gtk3/) [`glib`](../glib/) [`cairo`](../cairo/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`libgcrypt`](../libgcrypt/) [`libtasn1`](../libtasn1/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libepoxy`](../libepoxy/) [`libxml2`](../libxml2/) [`libpng`](../libpng/) [`sqlite`](../sqlite/) [`zlib`](../zlib/) [`libwebp`](../libwebp/) [`libxslt`](../libxslt/) [`libsoup`](../libsoup/) [`gstreamer`](../gstreamer/) [`gst-plugins-base`](../gst-plugins-base/) [`hyphen`](../hyphen/) [`woff2`](../woff2/) [`libx11`](../libx11/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`libxkbcommon`](../libxkbcommon/) [`at-spi2-core`](../at-spi2-core/) [`gst-plugins-bad`](../gst-plugins-bad/) [`lcms2`](../lcms2/) [`libavif`](../libavif/) [`libjxl`](../libjxl/) [`libsecret`](../libsecret/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gst-plugins-bad`](../gst-plugins-bad/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`hyphen`](../hyphen/) [`icu`](../icu/) [`lcms2`](../lcms2/) [`libavif`](../libavif/) [`libepoxy`](../libepoxy/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libjxl`](../libjxl/) [`libpng`](../libpng/) [`libsecret`](../libsecret/) [`libsoup`](../libsoup/) [`libtasn1`](../libtasn1/) [`libwebp`](../libwebp/) [`libxml2`](../libxml2/) [`libxslt`](../libxslt/) [`pango`](../pango/) [`sqlite`](../sqlite/) [`woff2`](../woff2/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add webkitgtk
apk add webkitgtk-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install webkitgtk`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild webkitgtk` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (86 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gtk3 glib cairo fontconfig freetype libgcrypt libtasn1 harfbuzz icu libjpeg-turbo libepoxy libxml2 libpng sqlite zlib libwebp libxslt libsoup gstreamer gst-plugins-base hyphen woff2 libx11 wayland wayland-protocols libxkbcommon at-spi2-core gst-plugins-bad lcms2 libavif libjxl libsecret
```

<details><summary>Full build chain (86)</summary>

[`aom`](../aom/) [`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dav1d`](../dav1d/) [`dbus`](../dbus/) [`expat`](../expat/) [`fdk-aac`](../fdk-aac/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gst-plugins-bad`](../gst-plugins-bad/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`highway`](../highway/) [`hyphen`](../hyphen/) [`icu`](../icu/) [`lcms2`](../lcms2/) [`libavif`](../libavif/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libjxl`](../libjxl/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libpsl`](../libpsl/) [`libsecret`](../libsecret/) [`libsoup`](../libsoup/) [`libtasn1`](../libtasn1/) [`libtheora`](../libtheora/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxslt`](../libxslt/) [`libxtst`](../libxtst/) [`libyuv`](../libyuv/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`openh264`](../openh264/) [`openssl`](../openssl/) [`opus`](../opus/) [`orc`](../orc/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`sqlite`](../sqlite/) [`svt-av1`](../svt-av1/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`woff2`](../woff2/) [`x265`](../x265/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://webkitgtk.org/releases/webkitgtk-2.44.4.tar.xz
mkdir -p dist src/webkitgtk
curl -fL -o dist/$(basename $URL) $URL
echo "2ce4ec1b78413035037aba8326b31ed72696626b7bea7bace5e46ac0d8cbe796  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/webkitgtk --strip-components=1
```

### 3. Apply the QNX patches

5 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/webkitgtk < ports/webkitgtk/patches/0001-cmake4-quote-framework-vars.patch
patch -p1 -d src/webkitgtk < ports/webkitgtk/patches/0002-icu76-cplusplus-header-api.patch
patch -p1 -d src/webkitgtk < ports/webkitgtk/patches/0003-gtk-qnx-memorypressure.patch
patch -p1 -d src/webkitgtk < ports/webkitgtk/patches/0004-qnx-structure-heap-128mb.patch
patch -p1 -d src/webkitgtk < ports/webkitgtk/patches/0005-glib-pidsocket-linux-only-arg.patch
```

**[`0001-cmake4-quote-framework-vars.patch`](patches/0001-cmake4-quote-framework-vars.patch)** — Quote the framework variables in _WEBKIT_TARGET_LINK_FRAMEWORK.

> Configuring with CMake 4 fails before any compilation:
>
>     CMake Error at Source/cmake/WebKitMacros.cmake:280 (if):
>       if given arguments:
>         "(" "NOT" "_linked_into" ")" "OR" "(" "WTF" "STREQUAL" ")" "OR" ...
>       Unknown arguments specified
>
> When a framework has no LINKED_INTO property, ${_linked_into} expands to
> nothing and STREQUAL loses its right-hand operand.  Older CMake accepted the
> malformed expression; 4.x does not.  Quoting both expansions makes the
> comparison well-formed for every CMake version, and changes nothing about what
> it decides.  Not QNX-specific.

Touches: `Source/cmake/WebKitMacros.cmake` · Upstream status: Pending

**[`0002-icu76-cplusplus-header-api.patch`](patches/0002-icu76-cplusplus-header-api.patch)** — Turn off ICU 76's second C++ API switch as well.

> WTF/Platform.h sets U_SHOW_CPLUSPLUS_API 0 because WebKit uses only ICU's C
> API.  ICU 76 introduced U_SHOW_CPLUSPLUS_HEADER_API, defaulting to 1, and moved
> some declarations behind "U_SHOW_CPLUSPLUS_API || U_SHOW_CPLUSPLUS_HEADER_API".
> Building against ICU 76 therefore compiles a std::u16string_view iterator in
> uset.h whose helper types the first switch has already suppressed:
>
>     unicode/uset.h:1660:21: error: 'ConstChar16Ptr' was not declared in this scope
>     unicode/uset.h:1763:5:  error: 'UnicodeString' does not name a type
>
> Setting both to 0 is what the ICU 76 release notes prescribe for C-only
> consumers.  Not QNX-specific: any WebKit 2.44 build against ICU >= 76 hits it.

Touches: `Source/WTF/wtf/Platform.h` · Upstream status: Pending

**[`0003-gtk-qnx-memorypressure.patch`](patches/0003-gtk-qnx-memorypressure.patch)** — GTK port: select the Unix memory-pressure handler on QNX.

> The same gap as the JSCOnly port has.  MemoryPressureHandler.h declares
> m_holdOffTimer and holdOffTimerFired() for "OS(LINUX) || OS(FREEBSD) ||
> OS(QNX)", and unix/MemoryPressureHandlerUnix.cpp defines them -- it already
> carries an OS(QNX) branch that reads the resident set size with
> devctl(DCMD_PROC_ASINFO) on /proc/self/ctl.  Only this platform list had no
> QNX case, so the build fell through to the generic handler, which does not
> define what the header declared, and WTF failed to link.

Touches: `Source/WTF/wtf/PlatformGTK.cmake` · Upstream status: QNX-specific

**[`0004-qnx-structure-heap-128mb.patch`](patches/0004-qnx-structure-heap-128mb.patch)** — Use a 128 MB structure heap on QNX.

> JSC reserves its structure heap with tryReserveUncommittedAligned(size,
> structureHeapAddressSize).  StructureMemoryManager halves the size up to eight
> times when the reservation fails, but the *alignment* argument stays at
> structureHeapAddressSize -- so with the 4 GB default every attempt needs up to
> 8 GB of contiguous address space.  On a QNX target with 1 GB of RAM all eight
> fail and the RELEASE_ASSERT aborts inside JSContextGroupCreate(), before any
> script runs.  It passes on an 8 GB board, so the failure looks like an
> architecture difference and is really an address-space one.
>
> 128 MB is the value PLAYSTATION already uses for the same reason.  StructureID
> derives its masks from this constant, so nothing else needs changing.

Touches: `Source/JavaScriptCore/runtime/StructureID.h` · Upstream status: QNX-specific

**[`0005-glib-pidsocket-linux-only-arg.patch`](patches/0005-glib-pidsocket-linux-only-arg.patch)** — Only pass the PID socket argument where it exists.

> ProcessLauncherGLib declares pidSocketPair/pidSocket under "#if OS(LINUX)" and
> guards the fd hand-off and the readPIDFromPeer() call the same way -- but puts
> the argument on the child's command line unconditionally:
>
>     ProcessLauncherGLib.cpp:205: error: 'pidSocket' was not declared in this scope
>
> The child agrees with the guards, not with the argument: AuxiliaryProcessMain
> reads argv[3] under "#if USE(GLIB) && OS(LINUX)".  The mechanism is Linux-only
> by nature -- it recovers a sandboxed child's pid across a PID namespace using
> SCM_CREDENTIALS, which QNX does not have -- and the non-Linux path already
> takes the pid from g_subprocess_get_identifier().  So the argument simply
> should not be pushed when the rest is compiled out.  Any non-Linux GLib port
> hits this.

Touches: `Source/WebKit/UIProcess/Launcher/glib/ProcessLauncherGLib.cpp` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/webkitgtk -B build/$QNX_ARCH/webkitgtk -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DPORT=GTK \
      -DUSE_GTK4=OFF \
      -DENABLE_JIT=OFF \
      -DENABLE_C_LOOP=ON \
      -DENABLE_FTL_JIT=OFF \
      -DENABLE_SAMPLING_PROFILER=OFF \
      -DENABLE_WEBASSEMBLY=OFF \
      -DUSE_GBM=OFF \
      -DUSE_LIBDRM=OFF \
      -DENABLE_WEBGL=OFF \
      -DENABLE_BUBBLEWRAP_SANDBOX=OFF \
      -DENABLE_GAMEPAD=OFF \
      -DENABLE_JOURNALD_LOG=OFF \
      -DENABLE_INTROSPECTION=OFF \
      -DENABLE_DOCUMENTATION=OFF \
      -DENABLE_MINIBROWSER=OFF \
      -DENABLE_API_TESTS=OFF \
      -DENABLE_SPELLCHECK=OFF \
      -DUSE_LIBHYPHEN=ON \
      -DUSE_WOFF2=ON \
      -DUSE_SYSTEM_MALLOC=ON \
      -DENABLE_VIDEO=ON \
      -DENABLE_WEB_AUDIO=ON \
      # \
      GStreamer's \
      GL \
      elements \
      need \
      EGL/GLES \
      and \
      libgstgl, \
      which \
      # \
      this \
      tree \
      has \
      no \
      way \
      to \
      build; \
      media \
      decodes \
      into \
      system \
      # \
      memory \
      instead \
      of \
      GL \
      textures. \
      -DUSE_GSTREAMER_GL=OFF \
      # \
      QNX \
      ships \
      its \
      own \
      <backtrace.h> \
      -- \
      its \
      native \
      backtrace \
      API, \
      # \
      nothing \
      to \
      do \
      with \
      GCC's \
      libbacktrace \
      -- \
      so \
      WebKit's \
      # \
      FindLibBacktrace \
      matches \
      the \
      header \
      name \
      and \
      enables \
      a \
      # \
      library \
      that \
      is \
      not \
      there: \
      StackTrace.cpp \
      then \
      fails \
      on \
      # \
      backtrace_create_state/backtrace_pcinfo/backtrace_syminfo. \
      -DUSE_LIBBACKTRACE=OFF \
      # \
      QNX's \
      only \
      EGL \
      belongs \
      to \
      Screen, \
      and \
      it \
      types \
      # \
      EGLNativeDisplayType \
      as \
      an \
      integer \
      handle. \
      WebKit's \
      X11 \
      and \
      # \
      Wayland \
      display \
      backends \
      pass \
      a \
      Display*/wl_display* \
      to \
      it: \
      # \
      invalid \
      conversion \
      from \
      'Display*' \
      to \
      'EGLNativeDisplayType' \
      # \
      There \
      is \
      no \
      Mesa \
      here \
      to \
      provide \
      an \
      X11- \
      or \
      Wayland-bound \
      EGL, \
      # \
      so \
      those \
      two \
      windowing \
      targets \
      cannot \
      work. \
      Everything \
      else \
      # \
      -- \
      the \
      engine, \
      DOM, \
      layout, \
      networking, \
      media \
      -- \
      does \
      not \
      # \
      depend \
      on \
      them. \
      A \
      browser \
      window \
      would \
      need \
      a \
      PlatformDisplay \
      # \
      backend \
      written \
      against \
      QNX \
      Screen, \
      which \
      is \
      new \
      development, \
      # \
      not \
      a \
      port. \
      -DENABLE_X11_TARGET=OFF \
      -DENABLE_WAYLAND_TARGET=OFF \
      # \
      gtest's \
      death-test \
      regex \
      uses \
      regcomp/regexec/regfree, \
      which \
      on \
      # \
      QNX \
      live \
      in \
      libregex, \
      not \
      libc. \
      OptionsJSCOnly.cmake \
      forces \
      # \
      ENABLE_API_TESTS \
      back \
      on \
      for \
      every \
      non-Windows \
      port, \
      so \
      gtest \
      is \
      # \
      built \
      whatever \
      -DENABLE_API_TESTS \
      says. \
      The \
      list \
      repeats \
      the \
      # \
      toolchain's \
      default \
      because \
      a \
      -D \
      replaces \
      it \
      rather \
      than \
      adding. \
      -DCMAKE_CXX_STANDARD_LIBRARIES="-lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify -lregex" \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_CXX_STANDARD=20
cmake --build build/$QNX_ARCH/webkitgtk -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/webkitgtk
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- 2.44 is the last series that still renders with Cairo: 2.46 switched the GTK port to Skia, which brings its own graphics stack and hard-requires GBM.
- Also linked at run time (found by scripts/gen-dependency-index): gst-plugins-bad lcms2 libavif libjxl libsecret
- What QNX cannot provide, and why each is off: USE_GBM / USE_LIBDRM  - no DRM/KMS on QNX; these are the buffer-sharing path for accelerated compositing between processes. ENABLE_WEBGL          - pulls in ANGLE and a real GL driver. BUBBLEWRAP_SANDBOX    - Linux namespaces and seccomp. ENABLE_GAMEPAD        - libmanette, which is udev-based. JOURNALD_LOG          - systemd. ENABLE_INTROSPECTION  - would have to run target binaries. GL entry points come through libepoxy, which resolves them at run time, so no GL library is linked here.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH webkitgtk`.

### 5. What it installs

284 files under `/usr/local`: 6 shared-library entries, 1 programs, 219 headers.

```
/usr/local/lib/libjavascriptcoregtk-4.1.so
/usr/local/lib/libjavascriptcoregtk-4.1.so.0
/usr/local/lib/libjavascriptcoregtk-4.1.so.0.5.8
/usr/local/lib/libwebkit2gtk-4.1.so
/usr/local/lib/libwebkit2gtk-4.1.so.0
/usr/local/lib/libwebkit2gtk-4.1.so.0.13.8
/usr/local/bin/WebKitWebDriver
```

The complete list is `stage/<arch>/.manifest/webkitgtk` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa webkitgtk
ls dists/8.0/*/ | grep '^webkitgtk-'
```

This writes `webkitgtk-<ver>-r0.tar.gz` (run time) and `webkitgtk-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add webkitgtk
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/webkitgtk-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/webkitgtk-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | webkitgtk 2.44.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/webkitgtk-2.44.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | webkitgtk 2.44.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/webkitgtk-2.44.4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.44.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add webkitgtk=<version>`), sources in `ported/webkitgtk/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates webkitgtk`; port one with `scripts/port-update webkitgtk`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.44.4
# 2.44 is the last series that still renders with Cairo: 2.46 switched the GTK
# port to Skia, which brings its own graphics stack and hard-requires GBM.
TARBALL=https://webkitgtk.org/releases/webkitgtk-2.44.4.tar.xz
SHA256=2ce4ec1b78413035037aba8326b31ed72696626b7bea7bace5e46ac0d8cbe796
# Also linked at run time (found by scripts/gen-dependency-index): gst-plugins-bad lcms2 libavif libjxl libsecret
DEPS="gtk3 glib cairo fontconfig freetype libgcrypt libtasn1 harfbuzz icu
      libjpeg-turbo libepoxy libxml2 libpng sqlite zlib libwebp libxslt
      libsoup gstreamer gst-plugins-base hyphen woff2 libx11 wayland
      wayland-protocols libxkbcommon at-spi2-core gst-plugins-bad lcms2 libavif libjxl libsecret"
BUILDSYS=cmake
# What QNX cannot provide, and why each is off:
#   USE_GBM / USE_LIBDRM  - no DRM/KMS on QNX; these are the buffer-sharing path
#                           for accelerated compositing between processes.
#   ENABLE_WEBGL          - pulls in ANGLE and a real GL driver.
#   BUBBLEWRAP_SANDBOX    - Linux namespaces and seccomp.
#   ENABLE_GAMEPAD        - libmanette, which is udev-based.
#   JOURNALD_LOG          - systemd.
#   ENABLE_INTROSPECTION  - would have to run target binaries.
# GL entry points come through libepoxy, which resolves them at run time, so no
# GL library is linked here.
CMAKE_ARGS_EXTRA=( -DPORT=GTK -DUSE_GTK4=OFF
                   -DENABLE_JIT=OFF -DENABLE_C_LOOP=ON -DENABLE_FTL_JIT=OFF
                   -DENABLE_SAMPLING_PROFILER=OFF -DENABLE_WEBASSEMBLY=OFF
                   -DUSE_GBM=OFF -DUSE_LIBDRM=OFF -DENABLE_WEBGL=OFF
                   -DENABLE_BUBBLEWRAP_SANDBOX=OFF -DENABLE_GAMEPAD=OFF
                   -DENABLE_JOURNALD_LOG=OFF -DENABLE_INTROSPECTION=OFF
                   -DENABLE_DOCUMENTATION=OFF -DENABLE_MINIBROWSER=OFF
                   -DENABLE_API_TESTS=OFF -DENABLE_SPELLCHECK=OFF
                   -DUSE_LIBHYPHEN=ON -DUSE_WOFF2=ON -DUSE_SYSTEM_MALLOC=ON
                   -DENABLE_VIDEO=ON -DENABLE_WEB_AUDIO=ON
                   # GStreamer's GL elements need EGL/GLES and libgstgl, which
                   # this tree has no way to build; media decodes into system
                   # memory instead of GL textures.
                   -DUSE_GSTREAMER_GL=OFF
                   # QNX ships its own <backtrace.h> -- its native backtrace API,
                   # nothing to do with GCC's libbacktrace -- so WebKit's
                   # FindLibBacktrace matches the header name and enables a
                   # library that is not there: StackTrace.cpp then fails on
                   # backtrace_create_state/backtrace_pcinfo/backtrace_syminfo.
                   -DUSE_LIBBACKTRACE=OFF
                   # QNX's only EGL belongs to Screen, and it types
                   # EGLNativeDisplayType as an integer handle.  WebKit's X11 and
                   # Wayland display backends pass a Display*/wl_display* to it:
                   #   invalid conversion from 'Display*' to 'EGLNativeDisplayType'
                   # There is no Mesa here to provide an X11- or Wayland-bound EGL,
                   # so those two windowing targets cannot work.  Everything else
                   # -- the engine, DOM, layout, networking, media -- does not
                   # depend on them.  A browser window would need a PlatformDisplay
                   # backend written against QNX Screen, which is new development,
                   # not a port.
                   -DENABLE_X11_TARGET=OFF -DENABLE_WAYLAND_TARGET=OFF
                   # gtest's death-test regex uses regcomp/regexec/regfree, which on
                   # QNX live in libregex, not libc.  OptionsJSCOnly.cmake forces
                   # ENABLE_API_TESTS back on for every non-Windows port, so gtest is
                   # built whatever -DENABLE_API_TESTS says.  The list repeats the
                   # toolchain's default because a -D replaces it rather than adding.
                   -DCMAKE_CXX_STANDARD_LIBRARIES="-lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify -lregex"
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                   -DCMAKE_CXX_STANDARD=20 )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
