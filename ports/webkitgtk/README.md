# webkitgtk 2.54.3

> WebKitGTK web content engine for GTK applications

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://webkitgtk.org/releases/webkitgtk-2.54.3.tar.xz |
| SHA-256 | `2779c1b46f88762e8400650271ba14452dc4e30b37059981552d8b562b7d0862` |
| Build system | CMake |
| QNX patches | 11 |
| Build dependencies | [`gtk3`](../gtk3/) [`glib`](../glib/) [`cairo`](../cairo/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`libgcrypt`](../libgcrypt/) [`libtasn1`](../libtasn1/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libepoxy`](../libepoxy/) [`libxml2`](../libxml2/) [`libpng`](../libpng/) [`sqlite`](../sqlite/) [`zlib`](../zlib/) [`libwebp`](../libwebp/) [`libxslt`](../libxslt/) [`libsoup`](../libsoup/) [`gstreamer`](../gstreamer/) [`gst-plugins-base`](../gst-plugins-base/) [`hyphen`](../hyphen/) [`woff2`](../woff2/) [`libx11`](../libx11/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`libxkbcommon`](../libxkbcommon/) [`at-spi2-core`](../at-spi2-core/) [`gst-plugins-bad`](../gst-plugins-bad/) [`lcms2`](../lcms2/) [`libavif`](../libavif/) [`libjxl`](../libjxl/) [`libsecret`](../libsecret/) [`expat`](../expat/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`hyphen`](../hyphen/) [`icu`](../icu/) [`lcms2`](../lcms2/) [`libavif`](../libavif/) [`libepoxy`](../libepoxy/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libjxl`](../libjxl/) [`libpng`](../libpng/) [`libsecret`](../libsecret/) [`libsoup`](../libsoup/) [`libtasn1`](../libtasn1/) [`libwebp`](../libwebp/) [`libxml2`](../libxml2/) [`libxslt`](../libxslt/) [`pango`](../pango/) [`sqlite`](../sqlite/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_webkitgtk`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `webkitgtk` (programs, shared libraries, run-time data) and `webkitgtk-dev` (headers, pkg-config/CMake files, static libraries; needs `webkitgtk`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`webkitgtk`](https://openqnx.github.io/oss/package/x86_64/webkitgtk.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/webkitgtk.html)) · [`webkitgtk-dev`](https://openqnx.github.io/oss/package/x86_64/webkitgtk-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/webkitgtk-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add webkitgtk` (dependencies come along); one version: `apk add webkitgtk=<version>` | `apk del webkitgtk` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install webkitgtk` (dependencies first) | `oss-get remove webkitgtk` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`webkitgtk-dev` the same way, by its own name (`apk add webkitgtk-dev`, `oss-get install webkitgtk-dev`, by hand with its own file and list `webkitgtk-dev.list`); remove it before `webkitgtk`, which it needs.

```sh
# on the QNX target, as root
apk add webkitgtk webkitgtk-dev
apk add qnx-test-webkitgtk && /usr/local/bin/qnx-port-tests/test_webkitgtk    # its on-target test, from tests-8.0-<arch>-m-z
apk del webkitgtk webkitgtk-dev qnx-test-webkitgtk
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/webkitgtk.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/webkitgtk.list
rm var/lib/manual/webkitgtk.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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

**Build the dependencies first** (91 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gtk3 glib cairo fontconfig freetype libgcrypt libtasn1 harfbuzz icu libjpeg-turbo libepoxy libxml2 libpng sqlite zlib libwebp libxslt libsoup gstreamer gst-plugins-base hyphen woff2 libx11 wayland wayland-protocols libxkbcommon at-spi2-core gst-plugins-bad lcms2 libavif libjxl libsecret expat
```

<details><summary>Full build chain (91)</summary>

[`aom`](../aom/) [`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dav1d`](../dav1d/) [`dbus`](../dbus/) [`expat`](../expat/) [`fdk-aac`](../fdk-aac/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gst-plugins-bad`](../gst-plugins-bad/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`highway`](../highway/) [`hyphen`](../hyphen/) [`icu`](../icu/) [`lcms2`](../lcms2/) [`libavif`](../libavif/) [`libdrm-headers`](../libdrm-headers/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libjxl`](../libjxl/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libpsl`](../libpsl/) [`libsecret`](../libsecret/) [`libsoup`](../libsoup/) [`libtasn1`](../libtasn1/) [`libtheora`](../libtheora/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxslt`](../libxslt/) [`libxtst`](../libxtst/) [`libyuv`](../libyuv/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`openh264`](../openh264/) [`openssl`](../openssl/) [`opus`](../opus/) [`orc`](../orc/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`svt-av1`](../svt-av1/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`woff2`](../woff2/) [`x265`](../x265/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://webkitgtk.org/releases/webkitgtk-2.54.3.tar.xz
mkdir -p dist src/webkitgtk
curl -fL -o dist/$(basename $URL) $URL
echo "2779c1b46f88762e8400650271ba14452dc4e30b37059981552d8b562b7d0862  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/webkitgtk --strip-components=1
```

### 3. Apply the QNX patches

11 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/webkitgtk < ports/webkitgtk/patches/0001-skia-qnx-platform.patch
patch -p1 -d src/webkitgtk < ports/webkitgtk/patches/0002-gstreamer-gl-only-with-use-gstreamer-gl.patch
patch -p1 -d src/webkitgtk < ports/webkitgtk/patches/0010-qnx-madvise-posix-madvise.patch
patch -p1 -d src/webkitgtk < ports/webkitgtk/patches/0011-qnx-wtf-memory-status.patch
patch -p1 -d src/webkitgtk < ports/webkitgtk/patches/0012-jsc-float16-isnan-libcxx.patch
patch -p1 -d src/webkitgtk < ports/webkitgtk/patches/0013-wtf-qnx-timerfd-extern-c.patch
patch -p1 -d src/webkitgtk < ports/webkitgtk/patches/0014-webcore-intpoint-scaled-not-constexpr.patch
patch -p1 -d src/webkitgtk < ports/webkitgtk/patches/0015-wtf-switchon-visitonevariant-clang-only.patch
patch -p1 -d src/webkitgtk < ports/webkitgtk/patches/0016-webkit-protocolhandler-renderbuffer-needs-libdrm.patch
patch -p1 -d src/webkitgtk < ports/webkitgtk/patches/0017-skia-build-pathdump-strikeref.patch
patch -p1 -d src/webkitgtk < ports/webkitgtk/patches/0018-webkit-backingstore-eglimage-needs-gbm.patch
```

**[`0001-skia-qnx-platform.patch`](patches/0001-skia-qnx-platform.patch)** — Skia: build for QNX as a Unix platform.

> WebKitGTK 2.46 and later render with the Skia copy in Source/ThirdParty/skia;
> Cairo is no longer an option for the GTK port.  Skia picks its platform in
> include/private/SkFeatures.h from compiler macros.  QNX's GCC defines __QNX__
> and __QNXNTO__ but not __unix__, so the chain ended in its last branch,
> SK_BUILD_FOR_MAC: SkSemaphore.cpp then includes <dispatch/dispatch.h>,
> SkAutoLocaleSetter.h <xlocale.h>, and the Mac font/memory code paths are
> selected.  QNX is POSIX, so SK_BUILD_FOR_UNIX is the right platform -- fonts
> come from fontconfig/FreeType as on Linux.
>
> SkAutoLocaleSetter (used when SkSL shaders are generated) calls
> newlocale()/uselocale()/freelocale(), which QNX's libc does not provide; it
> has the C locale only, which is what the setter switches to.  Treat QNX like
> Android/uClibc/newlib, where the setter is a no-op.

Touches: `Source/ThirdParty/skia/include/private/SkFeatures.h`, `Source/ThirdParty/skia/src/core/SkAutoLocaleSetter.h` · Upstream status: Pending

**[`0002-gstreamer-gl-only-with-use-gstreamer-gl.patch`](patches/0002-gstreamer-gl-only-with-use-gstreamer-gl.patch)** — Look for libgstgl only when USE_GSTREAMER_GL is on.

> GStreamerChecks.cmake puts "gl" in the required GStreamer components for every
> ENABLE_VIDEO build, so -DUSE_GSTREAMER_GL=OFF no longer configures when
> gstreamer-gl-1.0 is absent:
>
>     Could NOT find GStreamer (missing: GSTREAMER_GL_LIBRARIES)
>
> Nothing else needs it in that case: every <gst/gl/...> include and the
> GSTREAMER_GL_LIBRARIES link in WebCore/platform/GStreamer.cmake are already
> under USE(GSTREAMER_GL) / USE_GSTREAMER_GL, and the same file checks
> "USE_GSTREAMER_GL AND NOT PC_GSTREAMER_GL_FOUND" itself.  QNX has no
> EGL/GLES-backed libgstgl in this tree, so video decodes into system memory.

Touches: `Source/cmake/GStreamerChecks.cmake` · Upstream status: Pending

**[`0010-qnx-madvise-posix-madvise.patch`](patches/0010-qnx-madvise-posix-madvise.patch)** — bmalloc, WTF: madvise() through posix_madvise() on QNX.

> VMAllocate.h:327: error: 'madvise' was not declared in this scope
>     VMAllocate.h:327: error: 'MADV_DONTNEED' was not declared in this scope
>     VMAllocate.h:344: error: 'MADV_NORMAL' was not declared in this scope
>
> QNX's libc has no madvise() and no MADV_* constants; <sys/mman.h> offers the
> POSIX form, posix_madvise() with POSIX_MADV_NORMAL/WILLNEED/DONTNEED, which
> returns the error number instead of -1/errno.  bmalloc's VMAllocate.h (included
> even with USE_SYSTEM_MALLOC) and WTF's OSAllocatorPOSIX.cpp -- whose QNX
> branches upstream already call madvise(MADV_WILLNEED/MADV_DONTNEED) -- get a
> small static madvise() that forwards to posix_madvise() and sets errno, so the
> callers' "== -1 && errno == EAGAIN" loops are unchanged.  The block is the same
> in both files and guarded by !defined(MADV_NORMAL), so a unified source that
> includes both defines it once.  DONTNEED is advice here, as it is for every
> non-Linux caller: none of these paths rely on the pages reading back as zero
> (vmZeroAndPurge() re-maps with mmap(MAP_FIXED) on everything but Linux).

Touches: `Source/bmalloc/bmalloc/VMAllocate.h`, `Source/WTF/wtf/posix/OSAllocatorPOSIX.cpp` · Upstream status: Pending

**[`0011-qnx-wtf-memory-status.patch`](patches/0011-qnx-wtf-memory-status.patch)** — WTF: memoryStatus() on QNX.

> generic/MemoryFootprintGeneric.cpp:36: error: 'memoryStatus' was not
>         declared in this scope
>
> Both the GTK and the JSCOnly platform lists build
> generic/MemoryFootprintGeneric.cpp for QNX (alongside
> unix/MemoryPressureHandlerUnix.cpp), and since 2.46 that file computes the
> footprint through WTF::memoryStatus() -- which AvailableMemory.h declares only
> for iOS, Linux and FreeBSD.  Add QNX to those guards and give memoryStatus() a
> QNX branch: the resident set size from devctl(DCMD_PROC_ASINFO) on
> /proc/self/ctl, exactly how MemoryPressureHandlerUnix.cpp already measures the
> process on QNX; the percentage uses availableMemory() (sysconf
> _SC_PHYS_PAGES on QNX) like the other platforms.

Touches: `Source/WTF/wtf/AvailableMemory.h`, `Source/WTF/wtf/AvailableMemory.cpp` · Upstream status: Pending

**[`0012-jsc-float16-isnan-libcxx.patch`](patches/0012-jsc-float16-isnan-libcxx.patch)** — JSC: call std::isnan() on a double for Float16 typed arrays.

> JSGenericTypedArrayViewPrototypeFunctions.h:532: error: no matching
>         function for call to 'isnan(std::optional<WTF::Float16>::value_type&)'
>     c++/v1/__math/traits.h:111: error: no type named 'type' in
>         'struct std::enable_if<false, int>'
>
> QNX's C++ library is libc++, whose std::isnan() is a template restricted to
> arithmetic types (plus float/double/long double overloads that the class does
> not match exactly), so WTF::Float16 -- a class with an implicit conversion to
> double -- does not deduce.  libstdc++'s plain overloads take it through the
> conversion, which is why Linux builds pass.  includes() for Float16Array then
> fails to compile; convert explicitly, as the branch for the other float element
> sizes right below already does.  Same value, same result.

Touches: `Source/JavaScriptCore/runtime/JSGenericTypedArrayViewPrototypeFunctions.h` · Upstream status: Pending

**[`0013-wtf-qnx-timerfd-extern-c.patch`](patches/0013-wtf-qnx-timerfd-extern-c.patch)** — WTF: include QNX's <sys/timerfd.h> as a C header.

> RunLoopGLib.cpp: undefined reference to `timerfd_create(int, int)'
>     RunLoopGLib.cpp: undefined reference to `timerfd_settime(int, int,
>         itimerspec const*, itimerspec*)'
>
> The GLib run loop uses timerfd when CMake finds timerfd_create() (HAVE_TIMERFD),
> and on QNX it does: libtimerfd, linked by the toolchain's default libraries.
> But its <sys/timerfd.h> declares the functions without an extern "C" block, so
> C++ code references the mangled names and the link of libWTF fails.  Wrap the
> include in extern "C" on QNX.  (JSCOnly uses the generic run loop and does not
> build this file; the patch is carried in both ports to keep the shared tree the
> same.)

Touches: `Source/WTF/wtf/glib/RunLoopGLib.cpp` · Upstream status: Inappropriate [QNX system header lacks C++ guards]

**[`0014-webcore-intpoint-scaled-not-constexpr.patch`](patches/0014-webcore-intpoint-scaled-not-constexpr.patch)** — WebCore: IntPoint::scaled() is not constexpr with GCC and libc++.

> IntPoint.h:88: error: call to non-'constexpr' function
>         'long int std::__math::lround(float)'
>
> IntPoint::scaled() is declared constexpr and calls std::lround().  libstdc++
> lets GCC fold lround as a builtin; libc++ (the C++ library of QNX's GCC
> toolchain) declares it as an ordinary function, and GCC 12 -- which predates
> C++23's relaxed rule for constexpr functions (P2448) -- rejects a non-template
> constexpr function whose body can never be a constant expression.  Nothing
> uses scaled() in a constant expression, so drop the constexpr.

Touches: `Source/WebCore/platform/graphics/IntPoint.h` · Upstream status: Pending

**[`0015-wtf-switchon-visitonevariant-clang-only.patch`](patches/0015-wtf-switchon-visitonevariant-clang-only.patch)** — WTF: use the switch-based switchOn() for libc++ only with Clang.

> StyleTabSize.h:66: error: call to non-'constexpr' function
>         'decltype (WTF::visitOneVariant(...)) WTF::switchOn(V&&, F&& ...)'
>     (in 'constexpr bool WebCore::Style::TabSize::isZero() const')
>
> When the C++ library is libc++, StdLibExtras.h replaces switchOn() with a
> switch-based visitOneVariant() to work around code-size problems of libc++'s
> visit (webkit.org/b/279498).  That version is not constexpr; Clang in C++23
> mode accepts constexpr functions that call it (P2448), GCC 12 does not.  QNX's
> toolchain is exactly that pair -- GCC with libc++ -- so every constexpr caller
> in WebCore (TabSize::isZero() and the other style value types) fails.  Take the
> size workaround only with Clang; GCC + libc++ uses the generic constexpr
> switchOn() over WTF::visit, as GCC + libstdc++ already does.

Touches: `Source/WTF/wtf/StdLibExtras.h` · Upstream status: Pending

**[`0016-webkit-protocolhandler-renderbuffer-needs-libdrm.patch`](patches/0016-webkit-protocolhandler-renderbuffer-needs-libdrm.patch)** — webkitgtk: the "Buffer format" row without hardware acceleration needs USE(LIBDRM).

> renderBufferDescription() is defined inside #if USE(LIBDRM) in
> WebKitProtocolHandler.cpp, and the hardware-acceleration branch above only
> calls it under USE(LIBDRM). The other branch (acceleration off) calls it
> under PLATFORM(GTK) alone, so a GTK build without libdrm -- QNX has none --
> fails with "'renderBufferDescription' was not declared in this scope".
> Guard that call the same way.

Touches: `Source/WebKit/UIProcess/API/glib/WebKitProtocolHandler.cpp` · Upstream status: Pending

**[`0017-skia-build-pathdump-strikeref.patch`](patches/0017-skia-build-pathdump-strikeref.patch)** — skia: build SkPathDump.cpp and SkStrikeRef.cpp.

> Source/ThirdParty/skia/CMakeLists.txt leaves these two files out, but
> SkPathOpsDebug.cpp calls SkPath::dump(SkWStream*, bool) and SkFont.cpp
> calls SkStrikeRef(sk_sp<SkStrike>, float), which they define. Where the
> linker discards the unreferenced callers the omission goes unnoticed; the
> QNX link of libwebkit2gtk-4.1.so keeps them and stops with "undefined
> reference". Add the two files to the Skia source list.

Touches: `Source/ThirdParty/skia/CMakeLists.txt` · Upstream status: Pending

**[`0018-webkit-backingstore-eglimage-needs-gbm.patch`](patches/0018-webkit-backingstore-eglimage-needs-gbm.patch)** — webkitgtk: AcceleratedBackingStore::BufferEGLImage::create without GBM.

> DMABufBuffer::createEGLImage(GLDisplay&, const Attributes&) is defined
> inside #if USE(GBM) in DMABufBuffer.cpp, but
> AcceleratedBackingStore::BufferEGLImage::create calls it unconditionally,
> so a GTK build without GBM -- QNX has none -- fails to link
> libwebkit2gtk-4.1.so ("undefined reference to
> WebCore::DMABufBuffer::createEGLImage"). Without GBM no DMABuf buffer can
> reach this path; log and return no buffer there.

Touches: `Source/WebKit/UIProcess/gtk/AcceleratedBackingStore.cpp` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/webkitgtk; B=build/$QNX_ARCH/webkitgtk; mkdir -p $B
```

```bash
do_configure() {
  local HB="$B/host-unifdef"
  mkdir -p "$HB" || return 1
  ( unset CC CFLAGS CPPFLAGS LDFLAGS
    gcc -std=gnu99 -O2 -o "$HB/unifdef" "$S/Source/ThirdParty/unifdef/unifdef.c" ) \
    || { echo "native unifdef build failed"; return 1; }
  cmake -S "$S" -B "$B" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
    -DUSE_SYSTEM_UNIFDEF=ON -DUNIFDEF_EXECUTABLE="$HB/unifdef" \
    "${CMAKE_ARGS_EXTRA[@]}"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Since 2.46 the GTK port renders with Skia, built from the copy in Source/ThirdParty/skia (static, CPU raster here); Cairo is still linked, but only for GTK's own API.  GBM is optional: without it the web process draws into shared-memory buffers (or, with compositing off, paints without it).
- Also linked at run time (found by scripts/gen-dependency-index): gst-plugins-bad lcms2 libavif libjxl libsecret
- What QNX cannot provide, and why each is off: USE_GBM / USE_LIBDRM  - no DRM/KMS on QNX; these are the buffer-sharing path for accelerated compositing between processes. ENABLE_WEBGL          - pulls in ANGLE and a real GL driver. BUBBLEWRAP_SANDBOX    - Linux namespaces and seccomp. ENABLE_GAMEPAD        - libmanette, which is udev-based. JOURNALD_LOG          - systemd. ENABLE_INTROSPECTION  - would have to run target binaries. ENABLE_SPEECH_SYNTHESIS - needs Flite or libspiel, neither ported. USE_SYSPROF_CAPTURE   - Linux profiler tracing; the bundled copy needs <sys/syscall.h> (memfd_create), which QNX lacks. (ENABLE_GPU_PROCESS follows USE_GBM off by itself.) GL entry points come through libepoxy, which resolves them at run time, so no GL library is linked here.
- (No CMAKE_CXX_STANDARD here: OptionsCommon.cmake sets C++23 itself, and WebKitCommon.cmake requires GCC >= 12.2 -- exactly the SDP's compiler.)
- The GLib API headers are generated through unifdef.  USE_SYSTEM_UNIFDEF=OFF would build the bundled copy with the cross compiler -- a QNX binary the build cannot run -- and the build image has no unifdef.  Build the bundled source for the build machine instead and hand it to CMake.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH webkitgtk`.

### 5. What it installs

287 files under `/usr/local`: 6 shared-library entries, 1 programs, 221 headers.

```
/usr/local/lib/libjavascriptcoregtk-4.1.so
/usr/local/lib/libjavascriptcoregtk-4.1.so.0
/usr/local/lib/libjavascriptcoregtk-4.1.so.0.11.10
/usr/local/lib/libwebkit2gtk-4.1.so
/usr/local/lib/libwebkit2gtk-4.1.so.0
/usr/local/lib/libwebkit2gtk-4.1.so.0.24.6
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

### Test

[`test_webkitgtk`](../../tests/src/) links webkitgtk the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-webkitgtk`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-webkitgtk
/usr/local/bin/qnx-port-tests/test_webkitgtk
```

```
== webkitgtk ==
  ok   ...
PASS webkitgtk
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_webkitgtk` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_webkitgtk`](../../tests/src/) -- a program linked against webkitgtk the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - libwebkit2gtk-4.1 is 2.54.3
   - libjavascriptcoregtk-4.1 is 2.54
   - jsc_context_new
   - 6 * 7 = 42
   - recursion: fib(20) = 6765
   - strings and Array.sort
   - JSON.stringify
   - RegExp capture group
   - Date.UTC / toISOString (leap day)
   - Math.sqrt
   - C callback called from JS (twice)
   - object literal
   - object property read from C
   - property set from C, seen by JS
   - TypeError reported as a JSCException
   - exception name TypeError
   - scratch directory
   - libexec jsc runs a script file
   - jsc -e evaluates an expression
   - WebKitWebDriver --help
   - WebKitWebDriver listens on 127.0.0.1
   - GET /status -> HTTP 200
   - status: ready true
   - status: \"No sessions\"
   - unknown session -> 404 invalid session id

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | webkitgtk 2.44.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/webkitgtk-2.44.4-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | webkitgtk 2.44.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/webkitgtk-2.44.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | webkitgtk 2.44.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/webkitgtk-2.44.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | webkitgtk 2.44.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/webkitgtk-2.44.4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.54.3**. Kept from earlier: `2.44.4` ([recipe and patches](versions/2.44.4/)). Every version stays available: packages in `dists/8.0/<arch>/` (`apk add webkitgtk=<version>`), sources in `ported/webkitgtk/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates webkitgtk`; port one with `scripts/port-update webkitgtk`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.54.3
# Since 2.46 the GTK port renders with Skia, built from the copy in
# Source/ThirdParty/skia (static, CPU raster here); Cairo is still linked, but
# only for GTK's own API.  GBM is optional: without it the web process draws
# into shared-memory buffers (or, with compositing off, paints without it).
TARBALL=https://webkitgtk.org/releases/webkitgtk-2.54.3.tar.xz
SHA256=2779c1b46f88762e8400650271ba14452dc4e30b37059981552d8b562b7d0862
# Also linked at run time (found by scripts/gen-dependency-index): gst-plugins-bad lcms2 libavif libjxl libsecret
DEPS="gtk3 glib cairo fontconfig freetype libgcrypt libtasn1 harfbuzz icu
      libjpeg-turbo libepoxy libxml2 libpng sqlite zlib libwebp libxslt
      libsoup gstreamer gst-plugins-base hyphen woff2 libx11 wayland
      wayland-protocols libxkbcommon at-spi2-core gst-plugins-bad lcms2 libavif libjxl libsecret
      expat"
BUILDSYS=cmake
# What QNX cannot provide, and why each is off:
#   USE_GBM / USE_LIBDRM  - no DRM/KMS on QNX; these are the buffer-sharing path
#                           for accelerated compositing between processes.
#   ENABLE_WEBGL          - pulls in ANGLE and a real GL driver.
#   BUBBLEWRAP_SANDBOX    - Linux namespaces and seccomp.
#   ENABLE_GAMEPAD        - libmanette, which is udev-based.
#   JOURNALD_LOG          - systemd.
#   ENABLE_INTROSPECTION  - would have to run target binaries.
#   ENABLE_SPEECH_SYNTHESIS - needs Flite or libspiel, neither ported.
#   USE_SYSPROF_CAPTURE   - Linux profiler tracing; the bundled copy needs
#                           <sys/syscall.h> (memfd_create), which QNX lacks.
# (ENABLE_GPU_PROCESS follows USE_GBM off by itself.)
# GL entry points come through libepoxy, which resolves them at run time, so no
# GL library is linked here.
CMAKE_ARGS_EXTRA=( -DPORT=GTK -DUSE_GTK4=OFF
                   -DENABLE_JIT=OFF -DENABLE_C_LOOP=ON -DENABLE_FTL_JIT=OFF
                   -DENABLE_SAMPLING_PROFILER=OFF -DENABLE_WEBASSEMBLY=OFF
                   -DUSE_GBM=OFF -DUSE_LIBDRM=OFF -DENABLE_WEBGL=OFF
                   -DENABLE_BUBBLEWRAP_SANDBOX=OFF -DENABLE_GAMEPAD=OFF
                   -DENABLE_JOURNALD_LOG=OFF -DENABLE_INTROSPECTION=OFF
                   -DENABLE_SPEECH_SYNTHESIS=OFF -DUSE_SYSPROF_CAPTURE=OFF
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
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
# (No CMAKE_CXX_STANDARD here: OptionsCommon.cmake sets C++23 itself, and
# WebKitCommon.cmake requires GCC >= 12.2 -- exactly the SDP's compiler.)

# The GLib API headers are generated through unifdef.  USE_SYSTEM_UNIFDEF=OFF
# would build the bundled copy with the cross compiler -- a QNX binary the build
# cannot run -- and the build image has no unifdef.  Build the bundled source
# for the build machine instead and hand it to CMake.
do_configure() {
  local HB="$B/host-unifdef"
  mkdir -p "$HB" || return 1
  ( unset CC CFLAGS CPPFLAGS LDFLAGS
    gcc -std=gnu99 -O2 -o "$HB/unifdef" "$S/Source/ThirdParty/unifdef/unifdef.c" ) \
    || { echo "native unifdef build failed"; return 1; }
  cmake -S "$S" -B "$B" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
    -DUSE_SYSTEM_UNIFDEF=ON -DUNIFDEF_EXECUTABLE="$HB/unifdef" \
    "${CMAKE_ARGS_EXTRA[@]}"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
