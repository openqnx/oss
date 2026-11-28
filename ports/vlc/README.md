# vlc 3.0.24

> VLC media player and libvlc multimedia engine

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://code.videolan.org/videolan/vlc.git (tag `3.0.24`) |
| Build system | custom (see recipe) |
| QNX patches | 48 |
| Build dependencies | [`bzip2`](../bzip2/) [`libiconv`](../libiconv/) [`lz4`](../lz4/) [`openssl`](../openssl/) [`xz`](../xz/) [`zstd`](../zstd/) |
| Run-time dependencies | [`libiconv`](../libiconv/) [`xz`](../xz/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_vlc`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `vlc` (programs, shared libraries, run-time data) and `vlc-dev` (headers, pkg-config/CMake files, static libraries; needs `vlc`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`vlc`](https://openqnx.github.io/oss/package/x86_64/vlc.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/vlc.html)) · [`vlc-dev`](https://openqnx.github.io/oss/package/x86_64/vlc-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/vlc-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add vlc` (dependencies come along); one version: `apk add vlc=<version>` | `apk del vlc` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install vlc` (dependencies first) | `oss-get remove vlc` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`vlc-dev` the same way, by its own name (`apk add vlc-dev`, `oss-get install vlc-dev`, by hand with its own file and list `vlc-dev.list`); remove it before `vlc`, which it needs.

```sh
# on the QNX target, as root
apk add vlc vlc-dev
apk add qnx-test-vlc && /usr/local/bin/qnx-port-tests/test_vlc    # its on-target test, from tests-8.0-<arch>-m-z
apk del vlc vlc-dev qnx-test-vlc
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/vlc.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/vlc.list
rm var/lib/manual/vlc.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild vlc` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (9 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild bzip2 libiconv lz4 openssl xz zstd
```

<details><summary>Full build chain (9)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`lz4`](../lz4/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`xz`](../xz/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.0.24 --recurse-submodules --shallow-submodules \
    https://code.videolan.org/videolan/vlc.git src/vlc
```

### 3. Apply the QNX patches

48 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/vlc < ports/vlc/patches/0010-update-contrib-s-bootstrap-and-build-process-to-su.patch
patch -p1 -d src/vlc < ports/vlc/patches/0012-get-contrib-aom-working-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0013-get-contrib-asdcplib-working-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0014-disable-contrib-bluray-library-for-qnx-builds.patch
patch -p1 -d src/vlc < ports/vlc/patches/0015-disable-contrib-amf-for-qnx-builds.patch
patch -p1 -d src/vlc < ports/vlc/patches/0016-get-contrib-dvdnav-working-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0018-get-contrib-ffmpeg-building-for-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0019-get-contrib-fribidi-building-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0020-get-contrib-gcrypt-working-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0024-get-contrib-gpg-error-building-with-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0026-get-contrib-libarchive-building-with-qnx8.patch
patch -p1 -d src/vlc < ports/vlc/patches/0027-get-contrib-libass-building-for-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0029-get-contrib-microdns-building-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0030-get-contrib-modplug-building-against-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0031-get-contrib-mpcdec-building-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0032-get-contrib-nfs-building-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0034-get-contrib-protobuf-building-on-64bit-qnx-targets.patch
patch -p1 -d src/vlc < ports/vlc/patches/0035-get-contrib-qt-working-on-qnx8.patch
patch -p1 -d src/vlc < ports/vlc/patches/0036-get-contrib-lua-building-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0037-get-contrib-mpcdec-building-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0038-get-contrib-sidplay2-building-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0039-get-contrib-shout-building-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0040-get-contrib-srt-working-on-qnx8.patch
patch -p1 -d src/vlc < ports/vlc/patches/0041-get-contrib-taglib-working-on-qnx8.patch
patch -p1 -d src/vlc < ports/vlc/patches/0043-get-contrib-vpx-working-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0044-get-contrib-x264-working-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0045-get-contrib-x265-working-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0047-cleanup-use-of-am-cppflags.patch
patch -p1 -d src/vlc < ports/vlc/patches/0048-use-libpthread-in-makefiles-instead-of-lpthread.patch
patch -p1 -d src/vlc < ports/vlc/patches/0049-support-qt-builds-w-o-clipboard-support.patch
patch -p1 -d src/vlc < ports/vlc/patches/0050-fix-null-pointer-segfault-s-when-running-in-debug.patch
patch -p1 -d src/vlc < ports/vlc/patches/0051-replacement-for-strerror-l.patch
patch -p1 -d src/vlc < ports/vlc/patches/0052-fix-assert-when-enable-debug-is-on-and-newlocale-i.patch
patch -p1 -d src/vlc < ports/vlc/patches/0053-fix-build-when-hand-written-assembler-doesn-t-use.patch
patch -p1 -d src/vlc < ports/vlc/patches/0055-add-vlc-lib-path.patch
patch -p1 -d src/vlc < ports/vlc/patches/0056-qnx-support.patch
patch -p1 -d src/vlc < ports/vlc/patches/0057-audio-playback-locks-up-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0058-plug-vlc-s-build-into-qnx-recursive-build.patch
patch -p1 -d src/vlc < ports/vlc/patches/0059-fix-missing-dependencies-making-qt-plugin.patch
patch -p1 -d src/vlc < ports/vlc/patches/0061-qnx-screen-window.patch
patch -p1 -d src/vlc < ports/vlc/patches/0062-contrib-tremor-full-githash.patch
patch -p1 -d src/vlc < ports/vlc/patches/0063-contrib-qnx-ldflags-and-live555-compiler-quoting.patch
patch -p1 -d src/vlc < ports/vlc/patches/0064-contrib-qnx-live555-locale-opus-rtcd.patch
patch -p1 -d src/vlc < ports/vlc/patches/0065-contrib-qnx-asm-compiler-libmd-syslog-sa-restart.patch
patch -p1 -d src/vlc < ports/vlc/patches/0066-audio-output-alsa-qnx-iec958-rates.patch
patch -p1 -d src/vlc < ports/vlc/patches/0067-contrib-libass-no-clang-cl-flag-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0068-contrib-no-librist-on-qnx.patch
patch -p1 -d src/vlc < ports/vlc/patches/0069-contrib-x264-fseeko-on-qnx.patch
```

**[`0010-update-contrib-s-bootstrap-and-build-process-to-su.patch`](patches/0010-update-contrib-s-bootstrap-and-build-process-to-su.patch)** — Update contrib's bootstrap and build process to support

> QNX.
>
> This mostly involved recognizing a host triplet with 'nto' in it
> and then ensuring that an SDP has been sourced into the environment.
>
> Once this is verified, both the HAVE_QNX and HAVE_LINUX flags are set.
> QNX and Linux are often close enough that what works on Linux works on
> QNX. So to keep from having to touch everything HAVE_LINUX is set. In
> cases where something needs to handle different behaviour between Linux
> and QNX, it can switch on the HAVE_QNX flag.
>
> Finally QNX's cross compilers (qcc/q++) are set up with the required
> environment variables so that they know what target they are compiling
> for.
>
> Unfortunately, meson doesn't play nice with the QNX cross compilers. It
> doesn't know how to identify them so refuses to build anything. For
> meson, call the underlying toolchain directly instead of going through
> the qcc/q++ frontends.
>
> This commit does NOT actually make any updates to the individual contrib
> libraries. That will be happening in the following commits.

Touches: `contrib/bootstrap`, `contrib/src/gen-meson-crossfile.py`, `contrib/src/get-arch.sh`, `contrib/src/main.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0012-get-contrib-aom-working-on-qnx.patch`](patches/0012-get-contrib-aom-working-on-qnx.patch)** — Get contrib/aom working on QNX

> Two main changes to get contrib/aom to build/work on QNX.
> 1. No need to specify -lpthread. QNX doesn't have a separate pthread
>    library and asking to link against it won't work.
> 2. Disable the runtime CPU detection. It should still enable NEON if it
>    was detected during configuration.
>
> 3.0.24: aom 3.15.0 writes only ${CMAKE_THREAD_LIBS_INIT} into aom.pc (empty
> on QNX, where pthreads are in libc), so aom-qnx-pthreads.patch is dropped;
> the runtime CPU detection setting stays.

Touches: `contrib/src/aom/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0013-get-contrib-asdcplib-working-on-qnx.patch`](patches/0013-get-contrib-asdcplib-working-on-qnx.patch)** — Get contrib/asdcplib working on QNX

> Changes are:
> 1. q++, by default, builds against libc++. However that requires at
>    least c++11 (unless one uses clang apparently). asdcplib deliberately
>    sets the standard to gnu++98. Update that standard to gnu++11 so that
>    libc++ works.
>
> 2. Support QNX's /proc filesystem to discover the currently running
>    binary. This is needed by the libkumu source.
>
> 3. Explicitly link in libregex to get access to the various posix
>    regex APIs.
>
> 4. QNX has statvfs(), not statfs().
>
> 3.0.24: asdcplib 2_13_2 no longer forces -std=gnu++98, so the -std=gnu++11
> override is gone; qnx-support.patch is applied after the upstream patches
> and rewritten for 2_13_2 (configure.ac case and asdcplib.pc.in from the
> upstream pkg-config patch).

Touches: `contrib/src/asdcplib/0001-qnx-support.patch`, `contrib/src/asdcplib/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0014-disable-contrib-bluray-library-for-qnx-builds.patch`](patches/0014-disable-contrib-bluray-library-for-qnx-builds.patch)** — Disable contrib/bluray library for QNX builds

> The bluray library requires a JDK to build against and I'm not aware of
> one currently available for QNX. If one is found later it can be
> renabled.
>
> I did start on trying to get bluray to compile and it ran into a
> namespace conflict with the various _mutex_* APIs. So this also includes
> a patch to rename libbluray's internal _mutex_* APIs to _bd_mutex_*.

Touches: `contrib/src/bluray/0001-qnx-mutex-namespace-collision.patch`, `contrib/src/bluray/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0015-disable-contrib-amf-for-qnx-builds.patch`](patches/0015-disable-contrib-amf-for-qnx-builds.patch)** — Disable contrib/amf for QNX builds

> AMF provides an interface to HW decoders and it isn't plugged into QNX
> yet. Disable it for now.

Touches: `contrib/src/amf/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0016-get-contrib-dvdnav-working-on-qnx.patch`](patches/0016-get-contrib-dvdnav-working-on-qnx.patch)** — Get contrib/dvdnav working on QNX

> dvdnav wanted to add `-lpthread` when building on QNX. There is no
> libpthread so link failed.

Touches: `contrib/src/dvdnav/0001-qnx-support.patch`, `contrib/src/dvdnav/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0018-get-contrib-ffmpeg-building-for-qnx.patch`](patches/0018-get-contrib-ffmpeg-building-for-qnx.patch)** — Get contrib/ffmpeg building for QNX

> ffmpeg's build process needed to be taught how to build with qcc/q++.
> Mostly so that it knew the correct flags to pass to the compiler in
> order to get the preprocessor to generate the expected dependency files.
>
> In addition, when building for x86_64, the assembler didn't like some of
> the hand written assembly code. Turns out there was already a fix for
> that in ffmpeg's upstream, so I'm adding it as part of this change.
>
> 3.0.24: FFmpeg 8.1 already has the x86 mathops fix for binutils >= 2.41
> (FFmpeg effadce6), so its backport is dropped; the QNX qcc patch is applied
> after the upstream patches.

Touches: `contrib/src/ffmpeg/0001-qnx-support-qcc.patch`, `contrib/src/ffmpeg/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0019-get-contrib-fribidi-building-on-qnx.patch`](patches/0019-get-contrib-fribidi-building-on-qnx.patch)** — Get contrib/fribidi building on QNX

> fribidi's meson build adds the -ansi flag to the build if it detects
> that the compiler supports it. I'm not sure why.
>
> QNX8 no longer supports this option. Normally meson's compiler feature
> detection would pick up on this but because the meson build process
> in the docker doesn't work with qcc/q++, I have to specify the
> underlying compiler directly. That compiler _does_ support "-ansi" so it
> gets turned on.
>
> The build then fails when including stdlib.h because it makes use of
> "inline" which "-ansi" does not allow.
>
> Remove the usage of "-ansi" when QNX is in play.

Touches: `contrib/src/fribidi/0001-qnx-remove_ansi_compiler_flag.patch`, `contrib/src/fribidi/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0020-get-contrib-gcrypt-working-on-qnx.patch`](patches/0020-get-contrib-gcrypt-working-on-qnx.patch)** — Get contrib/gcrypt working on QNX

> When building QNX for an x86_64 target, gcrypt's configure process
> thinks that underscores get added to external symbols. This isn't true
> and causes some of the custom assembly code to not be found.
>
> Force configure to keep the underscore setting off. This is really only
> for x86_64 builds but is safe to apply to aarch64 builds as that also
> wants underscores to be off.

Touches: `contrib/src/gcrypt/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0024-get-contrib-gpg-error-building-with-qnx.patch`](patches/0024-get-contrib-gpg-error-building-with-qnx.patch)** — Get contrib/gpg-error building with QNX

> gpg-error needs to pass the -P flag to the preprocessor via -Wp,-P
> instead of directly to qcc, which assigns a different meaning to -P.
>
> Create a local header for QNX8 support that gets copied into gpg-error's
> source tree. It is specific to QNX8 though it would probably work fine
> for QNX 7.1. If/when support for QNX7.1 is required it can be dealt with
> then.
>
> 3.0.24: libgpg-error 1.61 is configured with --disable-threads, so its
> configure generates lock-obj-pub.native.h itself and the QNX lock-obj header
> (and the step that copied it) is gone; the -Wp,-P change is rewritten for
> 1.61's configure.ac and src/Makefile.am and applied after the upstream
> patches.

Touches: `contrib/src/gpg-error/0001-qnx-support.patch`, `contrib/src/gpg-error/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0026-get-contrib-libarchive-building-with-qnx8.patch`](patches/0026-get-contrib-libarchive-building-with-qnx8.patch)** — Get contrib/libarchive building with QNX8

> QNX8 has the lz4.h header in a subdirectory of sysroot. Update
> libarchive's build to deal with that.
>
> 3.0.24: libarchive 3.8.9 is built with CMake, so the configure.ac and source
> changes no longer apply; the same intent (find the SDP's lz4.h under
> usr/include/lz4/) is now a LZ4_INCLUDE_DIR setting.

Touches: `contrib/src/libarchive/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0027-get-contrib-libass-building-for-qnx.patch`](patches/0027-get-contrib-libass-building-for-qnx.patch)** — Get contrib/libass building for QNX

> libass needed to recognize QNX's host identifier so that it could
> configure how to run the assembler.
>
> 3.0.24: libass 0.17.5 matches $host_os instead of the host triplet, so the
> patch now adds nto-qnx* to that case (configure.ac and the shipped
> configure).

Touches: `contrib/src/ass/0001-qnx-support.patch`, `contrib/src/ass/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0029-get-contrib-microdns-building-on-qnx.patch`](patches/0029-get-contrib-microdns-building-on-qnx.patch)** — Get contrib/microdns building on QNX

> Take the unix codepaths when compiling for QNX.

Touches: `contrib/src/microdns/0001-qnx-is-like-unix.patch`, `contrib/src/microdns/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0030-get-contrib-modplug-building-against-qnx.patch`](patches/0030-get-contrib-modplug-building-against-qnx.patch)** — Get contrib/modplug building against qnx

> q++, by default, builds against libc++. However that requires at
> least c++11 (unless one uses clang apparently). modplug deliberately
> sets the standard to gnu++98.
>
> Update the standard used to c++11 so I can link against libc++ instead
> of libstdc++. I did it this way instead of just using libstdc++ because
> I didn't want to mix usage of libc++ and libstdc++ in the same
> application.
>
> This also required a change to the .pc file to specify libc++ instead of
> libstdc++.

Touches: `contrib/src/modplug/qnx-use-libc++.patch`, `contrib/src/modplug/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0031-get-contrib-mpcdec-building-on-qnx.patch`](patches/0031-get-contrib-mpcdec-building-on-qnx.patch)** — Get contrib/mpcdec building on QNX

> qcc/q++ does not support the '-pipe' argument. Don't use it when
> building using these tools.
>
> For what it is worth, qcc/q++ by default has the behaviour that -pipe
> enables in gcc/g++ so even if it was supported, it wouldn't change
> anything.

Touches: `contrib/src/mpcdec/0001-qnx-support.patch`, `contrib/src/mpcdec/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0032-get-contrib-nfs-building-on-qnx.patch`](patches/0032-get-contrib-nfs-building-on-qnx.patch)** — Get contrib/nfs building on QNX

> A handful of small changes to get the code compiling on QNX. Mostly
> dealing with what API to use
>
> 1. QNX's makedev takes three arguments, not two.
> 2. getsockname is available, so use it.
> 3. System level time APIs come from sys/time.h, not time.h
>
> 3.0.24: libnfs 6.0.2 includes <sys/time.h> on every non-Windows system, so
> qnx_use_sys_time.patch is dropped (point 3 above); qnx_use_getsockname.patch
> is rewritten for the new condition (which added __OpenBSD__); the patches
> are applied after the upstream ones and only for QNX.

Touches: `contrib/src/nfs/qnx_three_arg_makedev.patch`, `contrib/src/nfs/qnx_use_getsockname.patch`, `contrib/src/nfs/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0034-get-contrib-protobuf-building-on-64bit-qnx-targets.patch`](patches/0034-get-contrib-protobuf-building-on-64bit-qnx-targets.patch)** — Get contrib/protobuf building on 64bit QNX targets

> The version of protobuf in contrib didn't support a 64bit target on QNX.
> Add in the missing configuration for the required support.
>
> 3.0.24: protobuf 3.4.1: the sub-patch is rewritten for the new context (the
> QNX case now precedes _M_ARM).

Touches: `contrib/src/protobuf/0001-add-support-for-64-bit-QNX-builds.patch`, `contrib/src/protobuf/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0035-get-contrib-qt-working-on-qnx8.patch`](patches/0035-get-contrib-qt-working-on-qnx8.patch)** — Get contrib/qt working on QNX8

> A bunch of changes to get the contrib version of qt compiling and
> working on QNX8. Only minimal testing was done so improvements are
> likely possible.
>
> Changes are:
> 1. Different location of slog2.h header.
> 2. Make sure the top level window is a 'root' window. Otherwise any
>    dialog that pops up ends up being a detached floating window that
>    appears to be a separate application.
> 3. When a new native (screen) buffer is created, make sure screen's
>    blit'ter is done clearing it before QT tries to start rendering into
>    it.
> 4. Changes to how contrib drives qt's build so that the QNX specific
>    bits get exposed in a way that VLC can use them.
> 5. Correctly specify the QT target type based on the target type that
>    QT is cross compiling for.
> 6. Don't build the QT tools using the cross-compiler. They need to be
>    run on the development host, not the target.
>
> 3.0.24: Qt's rules.mak was reorganised (no separate cross build of the Qt
> tools any more, so change 6 is gone); the other changes are carried to their
> new places. Qt is not built by this port (--disable-qt).

Touches: `contrib/src/qt/0001-qnx-fix-slog2-support.patch`, `contrib/src/qt/0002-qnx-enable-root-window.patch`, `contrib/src/qt/0003-qnx-flush-clear.patch`, `contrib/src/qt/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0036-get-contrib-lua-building-on-qnx.patch`](patches/0036-get-contrib-lua-building-on-qnx.patch)** — Get contrib/lua building on QNX

> Treat QNX as a posix style target instead of a generic one.

Touches: `contrib/src/lua/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0037-get-contrib-mpcdec-building-on-qnx.patch`](patches/0037-get-contrib-mpcdec-building-on-qnx.patch)** — Get contrib/mpcdec building on QNX

> qcc/q++ does not support the '-pipe' argument. Don't use it when
> building using these tools.
>
> For what it is worth, qcc/q++ by default has the behaviour that -pipe
> enables in gcc/g++ so even if it was supported, it wouldn't change
> anything.

Touches: `contrib/src/samplerate/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0038-get-contrib-sidplay2-building-on-qnx.patch`](patches/0038-get-contrib-sidplay2-building-on-qnx.patch)** — Get contrib/sidplay2 building on QNX

> QNX8's compiler generates a narrowing warning when converting an 8bit
> hexadecimal value (eg: '\x81') to an int8_t. This breaks the build.
>
> The warning can be ignored as the bits in the int8_t are set correctly.

Touches: `contrib/src/sidplay2/0001-no-narrowing-warning.patch`, `contrib/src/sidplay2/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0039-get-contrib-shout-building-on-qnx.patch`](patches/0039-get-contrib-shout-building-on-qnx.patch)** — Get contrib/shout building on QNX

> shout has a bunch of helper functions named like _blah_*(). This
> clashes with some internal names from QNX's headers. For example,
> _mutex_lock().
>
> Rename all the clashing usages in shout from _blah_*() to
> shout_blah_*().
>
> 3.0.24: libshout 2.4.6: the same renames, regenerated against its thread.c.

Touches: `contrib/src/shout/0001-qnx-fix-namespace-clash.patch`, `contrib/src/shout/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0040-get-contrib-srt-working-on-qnx8.patch`](patches/0040-get-contrib-srt-working-on-qnx8.patch)** — Get contrib/srt working on QNX8

> Add a QNX target type that can be switched on. It is generally treated
> the same as the POSIX type but changes a few places where different
> header files need to be included.
>
> In addition, make sure that when static libraries are built, they can be
> built with -fpic so that VLC can turn them into shared libraries later.
>
> 3.0.24: srt 1.5.6: the QNX system type is rewritten for the new
> CMakeLists.txt; the PIC part (0002-qnx-static-pic.patch, ENABLE_STATIC_PIC)
> is dropped, because VLC's contrib CMake builds now pass
> -DCMAKE_POSITION_INDEPENDENT_CODE=ON (upstream ddbbf247).

Touches: `contrib/src/srt/0001-qnx-support.patch`, `contrib/src/srt/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0041-get-contrib-taglib-working-on-qnx8.patch`](patches/0041-get-contrib-taglib-working-on-qnx8.patch)** — Get contrib/taglib working on QNX8

> Two changes.
> 1. Update C++ standard from c++98 to C++11 in order to make it
>    compatible with QNX's libc++ implementation that requires C++11
>
> 2. Build static libraries with -fpic so that VLC can turn them into
>    shared objects later.
>
> 3.0.24: taglib 1.13.1 still sets CXX_STANDARD 98, so change 1 stays; change
> 2 (0002-qnx-use-fpic.patch) is dropped, because VLC's contrib CMake builds
> now pass -DCMAKE_POSITION_INDEPENDENT_CODE=ON (upstream ddbbf247).

Touches: `contrib/src/taglib/0001-use-c++11.patch`, `contrib/src/taglib/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0043-get-contrib-vpx-working-on-qnx.patch`](patches/0043-get-contrib-vpx-working-on-qnx.patch)** — Get contrib/vpx working on QNX

> Use qcc/q++ for cross compilation instead of the underlying gcc/g++
> compilers.
>
> Also disable runtime CPU detection for now. Need to figure out how to
> replace this functionality for QNX.

Touches: `contrib/src/vpx/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0044-get-contrib-x264-working-on-qnx.patch`](patches/0044-get-contrib-x264-working-on-qnx.patch)** — Get contrib/x264 working on QNX

> Using qcc/q++ as the pre-processor to generate dependency files requires
> passing the pre-processor specific arguments via '-Wp,<arg>'. Otherwise
> they get confused and the build fails.
>
> Also QNX has getopt_long so no need to disable that.

Touches: `contrib/src/x264/0001-updae-qnx-settings.patch`, `contrib/src/x264/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0045-get-contrib-x265-working-on-qnx.patch`](patches/0045-get-contrib-x265-working-on-qnx.patch)** — Get contrib/x265 working on QNX

> 1. Update C++ standard from c++98 to C++11 in order to make it
>    compatible with QNX's libc++ implementation that requires C++11
>
> 2. Build static libraries with -fpic so that VLC can turn them into
>    shared objects later.
>
> 3. QNX does not need `-lpthread` to be explicitly added to link in
>    pthread support. It breaks if you do so as there is no libpthread
>    available.
>
> 4. CMake will try to figure out what libraries are implicitly linked
>    when creating an executable/shared-object. The x265 CMakeLists will
>    then explicitly add these libraries to the .pc dependency lists. This
>    causes them to be explicitly linked later on when VLC creates the
>    final shared object.
>
>    One of this implicit libraries is the static version of libc (libcS)
>    and explicitly linking that in causes problems with malloc and
>    friends when a shared object is unloaded.
>
>    I'm not sure why x265 whats to explicitly link these in, it should
>    happen automatically. Perhaps it is trying to protect against a
>    different toolchain consuming what the x265 build produces? In any
>    case, there is only one toolchain on QNX so don't link in libcS to
>    avoid the malloc issue.
>
> 5. Update x265's build process so that it can properly pull the gcc
>    version from qcc/q++.
>
> 3.0.24: x265 4.1: x265-pkg-libs.patch is gone and upstream's 0010 patch now
> lets -lpthread into Libs.private, so point 3 moved into 0002-qnx-dont-link-
> libcS.patch (which now removes -lcS and -lpthread); the three sub-patches
> are regenerated against 4.1 with the upstream patches applied, and
> ENABLE_PIC is passed in X265_CONF.

Touches: `contrib/src/x265/0001-qnx-use-gnu++11.patch`, `contrib/src/x265/0002-qnx-dont-link-libcS.patch`, `contrib/src/x265/0003-qnx-get-compiler-version.patch`, `contrib/src/x265/rules.mak` · Upstream status: QNX-specific (from qnx-ports)

**[`0047-cleanup-use-of-am-cppflags.patch`](patches/0047-cleanup-use-of-am-cppflags.patch)** — Cleanup use of AM_CPPFLAGS

> A couple of modules needed to add an include path (-I) to their
> CPPFLAGS. They did this by modifying AM_CPPFLAGS which, unfortunately,
> impacted everyone else who used it. Gotta love make and its recursively
> expanded variables.
>
> This caused a problem because the 'access' module has its own
> screen/screen.h that would end up being picked up by the EGL module I
> was compiling for QNX instead of QNX's screen/screen.h
>
> Clean this up so that a module's Makefile.am uses a local variable
> instead of a global one. This required touching a bunch of #include paths
> as they were set with erroneous -I paths paths due to AM_CPPFLAGS being
> modified.

Touches: `modules/access/Makefile.am`, `modules/access_output/srt.c`, `modules/audio_filter/converter/tospdif.c`, `modules/codec/avcodec/video.c`, `modules/codec/Makefile.am`, `modules/codec/ttml/substtml.c` … · Upstream status: QNX-specific (from qnx-ports)

**[`0048-use-libpthread-in-makefiles-instead-of-lpthread.patch`](patches/0048-use-libpthread-in-makefiles-instead-of-lpthread.patch)** — Use $(LIBPTHREAD) in makefiles instead of -lpthread

> A handful of makefiles were explicitly adding -lpthread instead of using
> the $(LIBPTHREAD) variable. This caused problems on systems, like QNX,
> where there is no libpthread.

Touches: `modules/access/Makefile.am`, `modules/demux/Makefile.am`, `modules/video_output/Makefile.am` · Upstream status: QNX-specific (from qnx-ports)

**[`0049-support-qt-builds-w-o-clipboard-support.patch`](patches/0049-support-qt-builds-w-o-clipboard-support.patch)** — Support Qt builds w/o clipboard support

> Not all Qt targets have a clipboard. Support this situation by checking
> for support and disabling code as required.

Touches: `configure.ac`, `modules/gui/qt/dialogs/messages.cpp`, `modules/gui/qt/dialogs/openurl.cpp` · Upstream status: QNX-specific (from qnx-ports)

**[`0050-fix-null-pointer-segfault-s-when-running-in-debug.patch`](patches/0050-fix-null-pointer-segfault-s-when-running-in-debug.patch)** — Fix NULL-pointer segfault's when running in debug mode

> Ran into a few places where vlc would segfault when --enable-debug is
> enabled. The extra logging wasn't bothering to check if a string pointer
> was NULL or not.

Touches: `modules/demux/mp4/libmp4.c`, `src/input/demux.c` · Upstream status: QNX-specific (from qnx-ports)

**[`0051-replacement-for-strerror-l.patch`](patches/0051-replacement-for-strerror-l.patch)** — Replacement for strerror_l

> Add a replacement of strerror_l for systems that don't have it.
>
> Ignores the passed in local and just calls strerror(). A non-localized
> error message is better than no error message.

Touches: `compat/strerror_l.c`, `configure.ac`, `include/vlc_fixups.h` · Upstream status: QNX-specific (from qnx-ports)

**[`0052-fix-assert-when-enable-debug-is-on-and-newlocale-i.patch`](patches/0052-fix-assert-when-enable-debug-is-on-and-newlocale-i.patch)** — Fix assert when --enable-debug is on and newlocale

> isn't available
>
> On systems that don't have the uselocale/newlocale/duplocale/freelocal
> APIs, the fixups do nothing beyond return 0 (as appropriate).
> This worked fine until --enable-debug is turned on.
>
> There is at least one assert, in vlc_strerror_l, that will fire if
> newlocale returns 0 AND errno is neither ENOENT nor EINVAL. Since the
> newlocale() fixup didn't set errno whether the assert would fire or not
> was somewhat random.
>
> Rework the code a bit so that the newlocale fixup returns 0 with errno
> explicitly set to ENOENT. vlc_strerror_l will then just use strerror()
> instead of asserting. Any other failure will still trigger an assert.

Touches: `include/vlc_fixups.h`, `src/posix/error.c` · Upstream status: QNX-specific (from qnx-ports)

**[`0053-fix-build-when-hand-written-assembler-doesn-t-use.patch`](patches/0053-fix-build-when-hand-written-assembler-doesn-t-use.patch)** — Fix build when hand-written assembler doesn't use GOT

> Several of the contrib libraries (eg: libavcodec) have hand written
> assembler in them. While the c/c++ code is compiled with -fPIC, the hand
> written assembly generally isn't and doesn't bother trying to access
> symbols it wants via the GOT.
>
> This causes a problem when creating the final shared objects as the
> (non-GOT) relocation is not supported.
>
> Using SYMBOLIC_LDFLAGS adds -Bsymbolic to the link command line which
> in turn makes the linker treat global symbols that it finds as local
> and hence don't need the GOT.
>
> This does mean they can't be overriden by the dynamic linker but I
> believe that is ok in this instance.

Touches: `modules/codec/Makefile.am`, `modules/demux/Makefile.am`, `modules/packetizer/Makefile.am`, `modules/stream_out/Makefile.am`, `modules/video_chroma/Makefile.am` · Upstream status: QNX-specific (from qnx-ports)

**[`0055-add-vlc-lib-path.patch`](patches/0055-add-vlc-lib-path.patch)** — Add VLC_LIB_PATH

> Provide a way to override the hard coded VLC lib directory, set at
> compile time. Makes it easier to deal with situations where the location
> VLC is built/installed on the host is NOT the same as where it will be
> run on the target.

Touches: `src/posix/dirs.c` · Upstream status: QNX-specific (from qnx-ports)

**[`0056-qnx-support.patch`](patches/0056-qnx-support.patch)** — QNX support

> Get VLC building and working on QNX8. In theory it should work for 7.1
> as well, though I haven't tried.
>
> The main thing to note is that GLESv2 & EGL is used to display video
> output. With work Qt might have been able to supply a vout_window
> provider, or QNX specific vout_display and vout_window providers could
> have been written. I choose to use GLESv2 & EGL because I felt using a
> standardized API would be easier, and if there any fancy video effects
> they would probably be done via GL.

Touches: `configure.ac`, `include/vlc_vout_window.h`, `modules/gui/qt/components/interface_widgets.cpp`, `modules/gui/qt/qt.cpp`, `modules/misc/gnutls.c`, `modules/video_output/flaschen.c` … · Upstream status: QNX-specific (from qnx-ports)

**[`0057-audio-playback-locks-up-on-qnx.patch`](patches/0057-audio-playback-locks-up-on-qnx.patch)** — Audio playback locks up on QNX

> On QNX, sometimes audio playback would lock up. The easiest way to
> trigger this would be to start playback, then stop/restart it a few
> times.
>
> The problem was traced to how QNX deals with thread cancellation. It is
> a little different than Linux. Once a thread with a cancellation type of
> DEFERRED is cancelled, it _will_ be cancelled at the next cancellation
> point. Regardless of what the cancellation state of the thread is at the
> time the cancellation point is hit.
>
> The upswing is that if the audio_output's worker thread already had a
> deferred cancellation request when it set the cancellation state to
> DISABLED, it would get cancelled at the next cancellation point, while
> it held the mutex lock. Holding this lock would prevent future audio
> playback from working.
>
> Work around this by explicitly forcing a cancellation point every time
> the cancellation state is disabled. If cancellation is already disabled,
> the call correctly does nothing, so it really only has an effect when
> called when cancellation is currently enabled.
>
> This solution isn't foolproof. It is possible, though unlikely, that a
> cancellation could happen between the call to pthread_testcancel and the
> call to pthread_setcancelstate(DISABLED). Unfortunately I couldn't find
> another method to trigger a cancellation point that was lightweight and
> reliable.

Touches: `src/posix/thread.c` · Upstream status: QNX-specific (from qnx-ports)

**[`0058-plug-vlc-s-build-into-qnx-recursive-build.patch`](patches/0058-plug-vlc-s-build-into-qnx-recursive-build.patch)** — Plug VLC's build into QNX recursive build

> Provides required directory structure and Makefiles to allow VLC to be
> built via QNX's recursive build system.
>
> There is a linux-x86_64 target that does some work that is required to
> enable the actual target builds. Mostly downloading all the contrib
> packages and compiling a host version of protoc. The former is required
> to avoid concurrency conflicts when running with multiple make
> processes. The latter because the protoc that comes in modern Ubuntu
> installs (like the one in the docker image) is too new.

Touches: `qnx/.gitignore`, `qnx/Makefile`, `qnx/README.md`, `qnx/common.mk`, `qnx/linux-x86_64/.gitignore`, `qnx/linux-x86_64/Makefile` … · Upstream status: QNX-specific (from qnx-ports)

**[`0059-fix-missing-dependencies-making-qt-plugin.patch`](patches/0059-fix-missing-dependencies-making-qt-plugin.patch)** — Fix missing dependencies making QT plugin

> When trying to build VLC for x86_64 I ran into a problem making the QT
> plugin with missing dependencies. This led down a rabbit hole of trying
> to track dependencies and where they come from.
>
> The dependencies for a given plugin are discovered by configure by
> looking at the associated .pc file. In this case contrib's Qt5Gui.pc
> lists, among other things, fontconfig as a dependency. Thus automake
> adds -lfontconfig to the link command line for the QT plugin.
>
> fontconfig has its own dependencies, the relevant ones for this
> discussion are libxml2 and liblzma.
>
> At this point, libtool takes over. It looks at the various .la files
> that were created to find any transitive dependencies. It correctly
> identifies that libfontconfig.la wants libxml2 and liblzma, however
> since I'm making a shared object instead of a program it doesn't add
> them to the link. Thus any unresolved symbols from libfontconfig will
> likely remain unresolved, breaking the build.
>
> I don't think there is a way to get either libtool or configure to deal
> with this without major surgery. Instead I noticed that the Makefile.am
> that creates the QT plugin target adds the following variables to the
> link: QT_LIBS (which is set by configure), LIBS_qt (which appears to not
> be set by anyone and is empty) and LIBM.
>
> LIBS_qt, being empty, seems an obvious choice. So for the QNX build set
> it to hold the missing libraries (and anything THEY depend on).
>
> This got the build going for the QT plugin and luckily nothing else
> seems to have this problem. Ideally a global solution would be better
> but as long as this is the only failure of this kind, this specific
> solution is good enough.
>
> An interesting side note, this only breaks the x86_64 build. The
> aarch64le build, in theory, should have the same problem but it does
> not. For whatever reason, libfontconfig built for Arm64 doesn't seem to
> depend on the same unresolved symbols. This probably is worth further
> investigation when there is time.

Touches: `qnx/common.mk` · Upstream status: QNX-specific (from qnx-ports)

**[`0061-qnx-screen-window.patch`](patches/0061-qnx-screen-window.patch)** — A QNX Screen window provider, so the command-line player shows video

> The QNX EGL/GLES2 video output (egl_qnx, from qnx-ports) draws into a
> VOUT_WINDOW_TYPE_QNX window, and the only module that provided one was the Qt
> GUI, which this port does not build: "vlc -I dummy <file>" (or cvlc) decoded
> the video but no window appeared. qnx_window ("vout window") creates a
> top-level Screen window -- the video's size, centred, or the whole display
> for --fullscreen -- for egl_qnx + gles2 to draw into, and passes Screen's
> close and key events on to VLC (hotkeys).
>
> egl_qnx took the first EGL config eglChooseConfig() returned: Mesa (the
> virtio-gpu driver) lists the deepest first, RGBX1010102, which Screen cannot
> make window buffers of, and the SDP 8 EGL accepts a window only in its
> config's native visual ("can't create window buffers", EGL_BAD_NATIVE_WINDOW,
> "cannot create EGL window surface"). It now takes an 8-bit config with an
> RGBX/RGBA8888 visual and gives the window that format (recreating its
> buffers when the format changes).

Touches: `modules/video_output/Makefile.am`, `modules/video_output/opengl/egl.c`, `modules/video_output/qnx/window.c` · Upstream status: QNX-specific

**[`0062-contrib-tremor-full-githash.patch`](patches/0062-contrib-tremor-full-githash.patch)** — contrib: pin tremor by its full commit hash.

> tremor is fetched from git master and archived at TREMOR_HASH, and
> check_githash then reads the hash back from tremor-git.githash with a
> pattern for a 40-digit SHA-1. With the short TREMOR_HASH (b56ffce0) the
> pattern never matches, the comparison sees the whole line, and
> "make fetch-all" stops at .sum-tremor. Use the full hash of the same
> commit.

Touches: `contrib/src/tremor/rules.mak` · Upstream status: Pending

**[`0063-contrib-qnx-ldflags-and-live555-compiler-quoting.patch`](patches/0063-contrib-qnx-ldflags-and-live555-compiler-quoting.patch)** — contrib: keep qcc's -V out of the global LDFLAGS; quote live555's compilers.

> Two contribs stopped the QNX build of the 3.0.24 contrib tree:
>
> - ffmpeg/rules.mak appended -V$(QNX_ARCH) to LDFLAGS for ffmpeg's
>   configure, which links with the C compiler (qcc). LDFLAGS is one make
>   variable for all contribs, and main.mak now hands it to Meson as
>   c_link_args; the Meson contribs are built with the plain nto<arch>-gcc
>   (Meson does not know qcc), which rejects -V: dav1d's setup failed with
>   "Unable to detect linker ... unrecognized command-line option
>   '-Vgcc_ntox86_64'". Give the option to ffmpeg alone (--extra-ldflags).
>
> - live555/rules.mak passes C_COMPILER=$(CC) CPLUSPLUS_COMPILER=$(CXX) to
>   make unquoted. With CC = "qcc -V<target>" the second word became a make
>   option ("make: invalid option -- 'V'"). Quote both.

Touches: `contrib/src/ffmpeg/rules.mak`, `contrib/src/live555/rules.mak` · Upstream status: Pending

**[`0064-contrib-qnx-live555-locale-opus-rtcd.patch`](patches/0064-contrib-qnx-live555-locale-opus-rtcd.patch)** — contrib: live555 without newlocale() and opus without rtcd on QNX.

> live555 (the version of the 3.0.24 contribs) declares a locale_t member
> unless NEWLOCALE_NOT_USED is defined; the QNX build stopped with
> "locale_t does not name a type". Use its setlocale() path.
>
> opus on aarch64 was configured with run-time CPU detection, for which
> celt/arm/armcpu.c has no QNX method. Disable rtcd there, as the contrib
> already does for aarch64 Windows.

Touches: `contrib/src/live555/rules.mak`, `contrib/src/opus/rules.mak` · Upstream status: Pending

**[`0065-contrib-qnx-asm-compiler-libmd-syslog-sa-restart.patch`](patches/0065-contrib-qnx-asm-compiler-libmd-syslog-sa-restart.patch)** — contrib: QNX fixes for the aarch64 assembler, libarchive, zvbi and mpg123.

> The 3.0.24 contribs stopped on QNX in four places:
>
> - aarch64 .S files were assembled for x86_64. qcc and q++ take the target
>   from -V, which is part of $(CC)/$(CXX); CMake takes the ASM compiler from
>   the C compiler's path alone, and a bare qcc builds for x86_64 (mpg123:
>   "no such instruction: fmadd ..."). toolchain.cmake now names the ASM
>   compiler with its -V. x265 builds its arm64 .S files with custom commands
>   that use ${CMAKE_CXX_COMPILER} alone ("bad value 'armv8-a' for '-march='");
>   its ASM_FLAGS, which those commands carry, gets q++'s -V on aarch64.
> - libarchive: libmd can be on the compiler's default library path when it is
>   installed next to the SDP. The MD5Init link check then succeeds, but
>   find_library() is confined to the contrib prefix and the configure stops
>   with "LIBMD_LIBRARY ... NOTFOUND". libmd is not used (its digests are
>   optional for VLC's archive access).
> - zvbi: QNX has <syslog.h> but no <sys/syslog.h> (src/proxy-msg.h).
> - mpg123: QNX's <signal.h> has no SA_RESTART; the SIGILL probe for NEON in
>   getcpuflags_arm.c works without it.

Touches: `contrib/src/libarchive/rules.mak`, `contrib/src/main.mak`, `contrib/src/mpg123/rules.mak`, `contrib/src/x265/rules.mak`, `contrib/src/zvbi/rules.mak` · Upstream status: Pending

**[`0066-audio-output-alsa-qnx-iec958-rates.patch`](patches/0066-audio-output-alsa-qnx-iec958-rates.patch)** — audio_output/alsa: the IEC958 rate codes QNX's libasound lacks.

> QNX SDP 8.0 ships libasound with an ALSA-compatible API, so the ALSA audio
> output builds against it -- except that its <alsa/asoundef.h> defines only
> the 44.1, 48 and 32 kHz IEC958_AES3_CON_FS_* codes, and the S/PDIF pass-
> through setup uses eight more (22.05 to 192 kHz and NOTID). Define the
> missing ones with alsa-lib's values where the header has none.

Touches: `modules/audio_output/alsa.c` · Upstream status: Pending

**[`0067-contrib-libass-no-clang-cl-flag-on-qnx.patch`](patches/0067-contrib-libass-no-clang-cl-flag-on-qnx.patch)** — contrib: libass on QNX -- no clang-cl flag, no automake rerun.

> libass's configure tries -fno-math-errno and then the clang-cl spelling
> /clang:-fno-math-errno (AX_APPEND_COMPILE_FLAGS). The check only compiles
> (qcc -c), and qcc lets the extra argument through, so the flag was "accepted"
> and appended to CFLAGS. Every later check that links then failed on it as a
> missing input file: snprintf, strndup, lrint... and configure stopped with
> "Unable to locate math functions!". Give configure the answer for that check
> (its cache variable) on QNX.
>
> The contrib's QNX patch for libass changes configure.ac together with
> configure. configure.ac then is newer than aclocal.m4, and make reran
> aclocal-1.17 (the automake of the release) -- not on a build machine with
> automake 1.16: "aclocal-1.17: command not found". Touch the generated files
> after the patch.

Touches: `contrib/src/ass/rules.mak` · Upstream status: Pending

**[`0068-contrib-no-librist-on-qnx.patch`](patches/0068-contrib-no-librist-on-qnx.patch)** — contrib: no librist on QNX.

> librist is new among the contribs of 3.0.24 (3.0.21 had none) and serves
> the RIST access/output modules only. On QNX it does not build as it is:
> no <sys/endian.h> (QNX has the functions in <net/netbyte.h>); the
> contribs' Meson cross file names the system 'linux', so it compiles
> src/tun_linux.c, which is empty off Linux (an error under its
> -pedantic-errors) while rist.c still calls rist_tun_read/rist_tun_write;
> and its bundled cJSON references glibc's __isinf. Leave it out on QNX:
> VLC's configure then builds without the RIST modules.

Touches: `contrib/src/librist/rules.mak` · Upstream status: Inappropriate [QNX build choice]

**[`0069-contrib-x264-fseeko-on-qnx.patch`](patches/0069-contrib-x264-fseeko-on-qnx.patch)** — contrib: x264 uses fseeko on QNX, not fseeko64.

> x264's configure checks fseeko with "#define _FILE_OFFSET_BITS 64" as the
> second argument of cc_check -- the place of compiler flags. qcc passes the
> words on to the linker ("cannot find #define"), the check fails, and the
> next one picked fseeko64/ftello64, which QNX's libc does not provide: VLC's
> x264 plugin then failed to link (undefined reference to `fseeko64').
> Drop that argument on QNX; off_t is 64-bit there.

Touches: `contrib/src/x264/rules.mak` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/vlc; B=build/$QNX_ARCH/vlc; mkdir -p $B
```

```bash
do_patch() {
  cd "$SRCROOT" && ./bootstrap || return 1
  local h="$SRCROOT/qnx-host"; mkdir -p "$h/contrib"
  # VLC's contrib Makefile installs into $(PREFIX) when the environment sets
  # it ("PREFIX ?= ..."), and env.sh exports PREFIX=/usr/local for this tree's
  # own installs: the contribs then went to the build host's /usr/local
  # ("mkdir: cannot create directory /usr/local/share/aclocal"). Unset it.
  ( cd "$h/contrib" && "$SRCROOT/contrib/bootstrap" && env -u PREFIX make fetch-all ) || return 1
  # host protoc from the contrib protobuf (built with CMake since 3.0.22): the
  # contrib comments out libprotoc/protoc in cmake/CMakeLists.txt; put them
  # back and build only the protoc target for the build machine
  ( cd "$h/contrib" && env -u PREFIX make protobuf && cd protobuf && \
    sed -i -e 's,#include(libprotoc,include(libprotoc,' -e 's,#include(protoc,include(protoc,' cmake/CMakeLists.txt && \
    cmake -S cmake -B host-build -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_WITH_ZLIB=OFF \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_BUILD_TYPE=Release && \
    cmake --build host-build --target protoc -j"$(nproc)" && cp host-build/protoc "$h/protoc" ) || return 1
}
```

```bash
do_configure() {
  local h="$SRCROOT/qnx-host"
  mkdir -p "$B/contrib" && ( cd "$B/contrib" && \
    "$S/contrib/bootstrap" --host="$(_vlc_host)" --disable-qt --disable-qtsvg --disable-xcb --disable-sdl --disable-SDL_image && \
    env -u PREFIX -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_PATH \
      PROTOC="$h/protoc" CC="qcc -V$QCC_TARGET" CXX="q++ -V${QCC_TARGET}_cxx" \
      CFLAGS="-O2 -D_QNX_SOURCE" CXXFLAGS="-O2 -D_QNX_SOURCE" make -j"$(nproc)" $(_vlc_jobserver) ) || return 1
  # VLC itself, like its contribs, takes its libraries from the contrib tree
  # (--with-contrib sets the pkg-config path); env.sh's PKG_CONFIG_* must not
  # apply here either, or the plugins lose their include paths (mpeg2.h) --
  # and pkg-config must still be confined to the contrib tree, or it answers
  # from the build machine (libpulse.pc, sndio).
  mkdir -p "$B/build" && cd "$B/build" && \
  env -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH PKG_CONFIG_LIBDIR="$B/$(_vlc_host)/lib/pkgconfig" \
  CFLAGS="-D_QNX_SOURCE -O2" CXXFLAGS="-D_QNX_SOURCE -O2" CC="qcc -V$QCC_TARGET" CXX="q++ -V${QCC_TARGET}_cxx" \
  EGL_CFLAGS="-DDUMMY_EGL_CFLAG" EGL_LIBS="-lEGL -lscreen" \
  GLES2_CFLAGS="-DDUMMY_GLES2_CFLAG" GLES2_LIBS="-lEGL -lGLESv2 -lscreen" PROTOC="$h/protoc" \
  ALSA_CFLAGS=" " ALSA_LIBS="-lasound" \
  "$S/configure" --host="$(_vlc_host)" --build=x86_64-linux-gnu --prefix="$PREFIX" \
    --with-contrib="$B/contrib/../$(_vlc_host)" --disable-a52 --disable-xcb --disable-sdl-image --enable-gles2 \
    --disable-qt --disable-skins2 --disable-lua --disable-vdpau \
    --enable-alsa --disable-pulse --disable-sndio --disable-jack --enable-run-as-root
}
```

```bash
do_build() { env -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH PKG_CONFIG_LIBDIR="$B/$(_vlc_host)/lib/pkgconfig" make -C "$B/build" -j"$(nproc)" $(_vlc_jobserver); }
```

```bash
do_install() { make -C "$B/build" DESTDIR="$STAGE" install && \
  sed -i "s|-L$B/$(_vlc_host)/lib *||g" "$STAGE$PREFIX/lib/pkgconfig/vlc-plugin.pc" && \
  sed -i -E 's#^(Exec|TryExec)=(.*/)?vlc( |$)#\1=\2cvlc\3#' "$STAGE$PREFIX/share/applications/vlc.desktop"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- VLC 3.0.24 with qnx-ports' QNX support (patches/: a Screen + EGL/GLES2 video output, QNX fixes in the core and modules, and QNX recipes in VLC's contrib system). Built the way the fork's qnx/ makefiles do it: 1. ./bootstrap; fetch every contrib tarball (VLC's contrib system checks each one against its own SHA512 list); 2. build protoc natively from the contrib protobuf (the cross build of the protobuf contrib needs a matching protoc); 3. build the contribs for the target, then VLC against them. Audio output: the ALSA module, on the SDP's libasound (an ALSA-compatible API, playing through io-snd); patches/0066 defines the IEC958 rate codes its headers lack. There is no alsa.pc and pkg-config is confined to the contrib tree, so configure gets ALSA_CFLAGS/ALSA_LIBS directly. Pulse, sndio and JACK do not exist on QNX. The Qt GUI is left out (it needs a Qt 5 contrib build): libvlc, the plugins and the command-line player (cvlc) are built. Beyond its contribs, VLC links these components' shared libraries (found on the toolchain's default library path), so its package depends on them.
- The contribs find each other through the pkg-config paths VLC's contrib Makefile sets; env.sh's PKG_CONFIG_* (this tree's staging) must not apply -- its sysroot was prefixed to the contribs' own paths ("libopenjp2 >= 2.1.0 not found" for FFmpeg). SDL 1.2 and SDL_image are left out: they only serve the SDL image decoder module, and SDL_image's configure found no SDL.h for the contrib SDL 1.2. The contribs get -D_QNX_SOURCE through CFLAGS/CXXFLAGS (their Meson cross file and configure scripts take them from there): without it QNX's headers hide POSIX interfaces such as clock_gettime, and dav1d's configure stopped. The make that runs first in PATH may be the SDP's 4.4, which hands its children a "fifo:" jobserver; contribs built with CMake run gmake, which can be the build machine's 4.3 and stops on it ("invalid --jobserver-auth string"). The pipe style is understood by both.
- --enable-run-as-root: QNX targets usually run everything as root, and vlc otherwise stops with "VLC is not supposed to be run as root". Video on QNX Screen: egl_qnx + gles2 draw into a window from qnx_window (patches/0061), the Screen window provider for the command-line player.
- vlc-plugin.pc carries -L for the contrib tree in the build directory (where libintl/libiconv were found): meaningless on any other machine, and scripts/check-artifacts rejects build paths in installed metadata. vlc.desktop starts "vlc", which looks for a graphical interface module; none is built (the Qt interface needs Qt 5, skins2 needs X11), so from a menu or a file manager it only printed "no suitable interface module" and exited. Start cvlc there instead: it plays the file in a Screen window.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH vlc`.

### 5. What it installs

573 files under `/usr/local`: 4 shared-library entries, 4 programs, 98 headers.

```
/usr/local/lib/libvlc.so
/usr/local/lib/libvlc.so.11
/usr/local/lib/libvlccore.so
/usr/local/lib/libvlccore.so.9
/usr/local/bin/cvlc
/usr/local/bin/rvlc
/usr/local/bin/vlc
/usr/local/bin/vlc-wrapper
```

The complete list is `stage/<arch>/.manifest/vlc` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa vlc
ls dists/8.0/*/ | grep '^vlc-'
```

This writes `vlc-<ver>-r0.tar.gz` (run time) and `vlc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add vlc
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/vlc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/vlc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_vlc`](../../tests/src/) links vlc the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-vlc`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-vlc
/usr/local/bin/qnx-port-tests/test_vlc
```

```
== vlc ==
  ok   ...
PASS vlc
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_vlc` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_vlc`](../../tests/src/) -- a program linked against vlc the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - libvlc_get_version is 3.0.24
   - scratch directory
   - wrote 1 s 8 kHz mono WAV
   - libvlc_new
   - libvlc_media_new_path
   - parse request
   - media parsed
   - duration is 1000 ms
   - one audio track, 8000 Hz mono
   - libvlc_media_player_new_from_media
   - libvlc_media_player_play
   - playback reached the end
   - ~8000 samples reached the audio callback
   - decoded peak is the generated 12000
   - vlc -I dummy --aout=afile exits 0
   - afile wrote an 8 kHz mono WAV
   - afile output has a RIFF/WAVE header
   - the vlc program decoded ~8000 samples
   - PCM passed through unchanged

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | vlc 3.0.21-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/vlc-3.0.21-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | vlc 3.0.21-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/vlc-3.0.21-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | vlc 3.0.21-r1 | ✅ PASS | 2026-11-03 | [log](../../tests/logs/aarch64le/vlc-3.0.21-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | vlc 3.0.21-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/vlc-3.0.21-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | vlc 3.0.21-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/vlc-3.0.21-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | vlc 3.0.21-r2 | ✅ PASS | 2026-11-03 | [log](../../tests/logs/x86_64/vlc-3.0.21-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.0.24**. Kept from earlier: `3.0.21` ([recipe and patches](versions/3.0.21/)). Every version stays available: packages in `dists/8.0/<arch>/` (`apk add vlc=<version>`), sources in `ported/vlc/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates vlc`; port one with `scripts/port-update vlc`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.0.24; GIT=https://code.videolan.org/videolan/vlc.git; REF=3.0.24
# VLC 3.0.24 with qnx-ports' QNX support (patches/: a Screen + EGL/GLES2 video
# output, QNX fixes in the core and modules, and QNX recipes in VLC's contrib
# system). Built the way the fork's qnx/ makefiles do it:
#   1. ./bootstrap; fetch every contrib tarball (VLC's contrib system checks
#      each one against its own SHA512 list);
#   2. build protoc natively from the contrib protobuf (the cross build of the
#      protobuf contrib needs a matching protoc);
#   3. build the contribs for the target, then VLC against them.
# Audio output: the ALSA module, on the SDP's libasound (an ALSA-compatible
# API, playing through io-snd); patches/0066 defines the IEC958 rate codes
# its headers lack. There is no alsa.pc and pkg-config is confined to the
# contrib tree, so configure gets ALSA_CFLAGS/ALSA_LIBS directly. Pulse,
# sndio and JACK do not exist on QNX.
# The Qt GUI is left out (it needs a Qt 5 contrib build): libvlc, the plugins
# and the command-line player (cvlc) are built.
# Beyond its contribs, VLC links these components' shared libraries (found on
# the toolchain's default library path), so its package depends on them.
DEPS="bzip2 libiconv lz4 openssl xz zstd"
BUILDSYS=custom
_vlc_host() { echo "$QNX_TRIPLE"; }
do_patch() {
  cd "$SRCROOT" && ./bootstrap || return 1
  local h="$SRCROOT/qnx-host"; mkdir -p "$h/contrib"
  # VLC's contrib Makefile installs into $(PREFIX) when the environment sets
  # it ("PREFIX ?= ..."), and env.sh exports PREFIX=/usr/local for this tree's
  # own installs: the contribs then went to the build host's /usr/local
  # ("mkdir: cannot create directory /usr/local/share/aclocal"). Unset it.
  ( cd "$h/contrib" && "$SRCROOT/contrib/bootstrap" && env -u PREFIX make fetch-all ) || return 1
  # host protoc from the contrib protobuf (built with CMake since 3.0.22): the
  # contrib comments out libprotoc/protoc in cmake/CMakeLists.txt; put them
  # back and build only the protoc target for the build machine
  ( cd "$h/contrib" && env -u PREFIX make protobuf && cd protobuf && \
    sed -i -e 's,#include(libprotoc,include(libprotoc,' -e 's,#include(protoc,include(protoc,' cmake/CMakeLists.txt && \
    cmake -S cmake -B host-build -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_WITH_ZLIB=OFF \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_BUILD_TYPE=Release && \
    cmake --build host-build --target protoc -j"$(nproc)" && cp host-build/protoc "$h/protoc" ) || return 1
}
# The contribs find each other through the pkg-config paths VLC's contrib
# Makefile sets; env.sh's PKG_CONFIG_* (this tree's staging) must not apply --
# its sysroot was prefixed to the contribs' own paths ("libopenjp2 >= 2.1.0
# not found" for FFmpeg).
# SDL 1.2 and SDL_image are left out: they only serve the SDL image decoder
# module, and SDL_image's configure found no SDL.h for the contrib SDL 1.2.
# The contribs get -D_QNX_SOURCE through CFLAGS/CXXFLAGS (their Meson cross
# file and configure scripts take them from there): without it QNX's headers
# hide POSIX interfaces such as clock_gettime, and dav1d's configure stopped.
# The make that runs first in PATH may be the SDP's 4.4, which hands its
# children a "fifo:" jobserver; contribs built with CMake run gmake, which can
# be the build machine's 4.3 and stops on it ("invalid --jobserver-auth
# string"). The pipe style is understood by both.
_vlc_jobserver() { make --help 2>/dev/null | grep -q -- '--jobserver-style' && echo --jobserver-style=pipe; }
do_configure() {
  local h="$SRCROOT/qnx-host"
  mkdir -p "$B/contrib" && ( cd "$B/contrib" && \
    "$S/contrib/bootstrap" --host="$(_vlc_host)" --disable-qt --disable-qtsvg --disable-xcb --disable-sdl --disable-SDL_image && \
    env -u PREFIX -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_PATH \
      PROTOC="$h/protoc" CC="qcc -V$QCC_TARGET" CXX="q++ -V${QCC_TARGET}_cxx" \
      CFLAGS="-O2 -D_QNX_SOURCE" CXXFLAGS="-O2 -D_QNX_SOURCE" make -j"$(nproc)" $(_vlc_jobserver) ) || return 1
  # VLC itself, like its contribs, takes its libraries from the contrib tree
  # (--with-contrib sets the pkg-config path); env.sh's PKG_CONFIG_* must not
  # apply here either, or the plugins lose their include paths (mpeg2.h) --
  # and pkg-config must still be confined to the contrib tree, or it answers
  # from the build machine (libpulse.pc, sndio).
  mkdir -p "$B/build" && cd "$B/build" && \
  env -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH PKG_CONFIG_LIBDIR="$B/$(_vlc_host)/lib/pkgconfig" \
  CFLAGS="-D_QNX_SOURCE -O2" CXXFLAGS="-D_QNX_SOURCE -O2" CC="qcc -V$QCC_TARGET" CXX="q++ -V${QCC_TARGET}_cxx" \
  EGL_CFLAGS="-DDUMMY_EGL_CFLAG" EGL_LIBS="-lEGL -lscreen" \
  GLES2_CFLAGS="-DDUMMY_GLES2_CFLAG" GLES2_LIBS="-lEGL -lGLESv2 -lscreen" PROTOC="$h/protoc" \
  ALSA_CFLAGS=" " ALSA_LIBS="-lasound" \
  "$S/configure" --host="$(_vlc_host)" --build=x86_64-linux-gnu --prefix="$PREFIX" \
    --with-contrib="$B/contrib/../$(_vlc_host)" --disable-a52 --disable-xcb --disable-sdl-image --enable-gles2 \
    --disable-qt --disable-skins2 --disable-lua --disable-vdpau \
    --enable-alsa --disable-pulse --disable-sndio --disable-jack --enable-run-as-root
}
# --enable-run-as-root: QNX targets usually run everything as root, and vlc
# otherwise stops with "VLC is not supposed to be run as root".
# Video on QNX Screen: egl_qnx + gles2 draw into a window from qnx_window
# (patches/0061), the Screen window provider for the command-line player.
do_build() { env -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH PKG_CONFIG_LIBDIR="$B/$(_vlc_host)/lib/pkgconfig" make -C "$B/build" -j"$(nproc)" $(_vlc_jobserver); }
# vlc-plugin.pc carries -L for the contrib tree in the build directory (where
# libintl/libiconv were found): meaningless on any other machine, and
# scripts/check-artifacts rejects build paths in installed metadata.
# vlc.desktop starts "vlc", which looks for a graphical interface module; none
# is built (the Qt interface needs Qt 5, skins2 needs X11), so from a menu or
# a file manager it only printed "no suitable interface module" and exited.
# Start cvlc there instead: it plays the file in a Screen window.
do_install() { make -C "$B/build" DESTDIR="$STAGE" install && \
  sed -i "s|-L$B/$(_vlc_host)/lib *||g" "$STAGE$PREFIX/lib/pkgconfig/vlc-plugin.pc" && \
  sed -i -E 's#^(Exec|TryExec)=(.*/)?vlc( |$)#\1=\2cvlc\3#' "$STAGE$PREFIX/share/applications/vlc.desktop"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
