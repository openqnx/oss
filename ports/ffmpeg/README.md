# ffmpeg 7.1.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/FFmpeg/FFmpeg.git (tag `n7.1.1`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) [`bzip2`](../bzip2/) [`xz`](../xz/) [`opus`](../opus/) [`libvorbis`](../libvorbis/) [`libogg`](../libogg/) [`dav1d`](../dav1d/) [`openh264`](../openh264/) [`x264`](../x264/) [`x265`](../x265/) [`libvpx`](../libvpx/) [`libtheora`](../libtheora/) [`lame`](../lame/) |
| Run-time dependencies | [`bzip2`](../bzip2/) [`dav1d`](../dav1d/) [`lame`](../lame/) [`libtheora`](../libtheora/) [`libvorbis`](../libvorbis/) [`libvpx`](../libvpx/) [`openh264`](../openh264/) [`opus`](../opus/) [`x264`](../x264/) [`x265`](../x265/) [`xz`](../xz/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_ffmpeg`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install ffmpeg
oss-get install ffmpeg-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ffmpeg` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (17 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib bzip2 xz opus libvorbis libogg dav1d openh264 x264 x265 libvpx libtheora lame
```

<details><summary>Full build chain (17)</summary>

[`bzip2`](../bzip2/) [`dav1d`](../dav1d/) [`gettext`](../gettext/) [`lame`](../lame/) [`libiconv`](../libiconv/) [`libogg`](../libogg/) [`libtheora`](../libtheora/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libvpx`](../libvpx/) [`ncurses`](../ncurses/) [`openh264`](../openh264/) [`opus`](../opus/) [`x264`](../x264/) [`x265`](../x265/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b n7.1.1 --recurse-submodules --shallow-submodules \
    https://github.com/FFmpeg/FFmpeg.git src/ffmpeg
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/ffmpeg; B=build/$QNX_ARCH/ffmpeg; mkdir -p $B
```

```bash
do_configure() {
  local arch extra=()
  case "$QNX_ARCH" in
    aarch64le) arch=aarch64 ;;
    x86_64)    arch=x86_64
               # nasm now comes from hosttools/, so the x86 assembly builds.
               command -v nasm >/dev/null || extra+=( --disable-x86asm ) ;;
  esac
  cd "$B"
  "$S/configure" \
    --prefix="$PREFIX" \
    --enable-cross-compile --target-os=qnx --arch="$arch" \
    --cross-prefix="$QNX_HOST/usr/bin/$QNX_TRIPLE-" \
    --sysroot="$QNX_TARGET" \
    `# with --cross-prefix set, configure looks for <prefix>pkg-config and,
     # not finding one, silently falls back to "false" -- every pkg-config
     # probe then fails with "not found using pkg-config".` \
    --pkg-config=pkg-config \
    --extra-cflags="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
    --extra-ldflags="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -L$QNX_TARGET/$QNX_ARCH/usr/lib -L$QNX_TARGET/$QNX_ARCH/lib -shared-libgcc" \
    --extra-libs="-lsocket -lm" \
    --enable-shared --disable-static \
    --enable-gpl --enable-version3 \
    --enable-zlib --enable-bzlib --enable-lzma \
    --enable-libopus --enable-libvorbis \
    --enable-libdav1d --enable-libopenh264 --enable-libx264 --enable-libx265 \
    --enable-libvpx --enable-libtheora --enable-libmp3lame \
    --disable-doc --disable-debug \
    --disable-vaapi --disable-vdpau --disable-xlib --disable-libxcb \
    --disable-sdl2 --disable-alsa --disable-libdrm \
    "${extra[@]}"
}
```

```bash
do_build()   { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- FFmpeg has its own configure with a qnx target-os case; it wants the cross prefix rather than CC/CXX, and it must run from the build directory.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ffmpeg`.

### 5. What it installs

207 files under `/usr/local`: 24 shared-library entries, 2 programs, 141 headers.

```
/usr/local/lib/libavcodec.so
/usr/local/lib/libavcodec.so.61
/usr/local/lib/libavcodec.so.61.19.101
/usr/local/lib/libavdevice.so
/usr/local/lib/libavdevice.so.61
/usr/local/lib/libavdevice.so.61.3.100
/usr/local/lib/libavfilter.so
/usr/local/lib/libavfilter.so.10
/usr/local/lib/libavfilter.so.10.4.100
/usr/local/lib/libavformat.so
/usr/local/lib/libavformat.so.61
/usr/local/lib/libavformat.so.61.7.100
/usr/local/lib/libavutil.so
/usr/local/lib/libavutil.so.59
/usr/local/lib/libavutil.so.59.39.100
/usr/local/lib/libpostproc.so
...
```

The complete list is `stage/<arch>/.manifest/ffmpeg` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH ffmpeg
ls repo/8.0/*/ | grep '^ffmpeg-'
```

This writes `ffmpeg-<ver>-r0.tar.gz` (run time) and `ffmpeg-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install ffmpeg
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/ffmpeg-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/ffmpeg-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_ffmpeg`](../../tests/src/) links ffmpeg the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_ffmpeg root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_ffmpeg'
```

```
== ffmpeg ==
  ok   ...
PASS ffmpeg
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=7.1.1; GIT=https://github.com/FFmpeg/FFmpeg.git; REF=n7.1.1
DEPS="zlib bzip2 xz opus libvorbis libogg dav1d openh264 x264 x265 libvpx libtheora lame"
BUILDSYS=custom
# FFmpeg has its own configure with a qnx target-os case; it wants the cross
# prefix rather than CC/CXX, and it must run from the build directory.
do_configure() {
  local arch extra=()
  case "$QNX_ARCH" in
    aarch64le) arch=aarch64 ;;
    x86_64)    arch=x86_64
               # nasm now comes from hosttools/, so the x86 assembly builds.
               command -v nasm >/dev/null || extra+=( --disable-x86asm ) ;;
  esac
  cd "$B"
  "$S/configure" \
    --prefix="$PREFIX" \
    --enable-cross-compile --target-os=qnx --arch="$arch" \
    --cross-prefix="$QNX_HOST/usr/bin/$QNX_TRIPLE-" \
    --sysroot="$QNX_TARGET" \
    `# with --cross-prefix set, configure looks for <prefix>pkg-config and,
     # not finding one, silently falls back to "false" -- every pkg-config
     # probe then fails with "not found using pkg-config".` \
    --pkg-config=pkg-config \
    --extra-cflags="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
    --extra-ldflags="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -L$QNX_TARGET/$QNX_ARCH/usr/lib -L$QNX_TARGET/$QNX_ARCH/lib -shared-libgcc" \
    --extra-libs="-lsocket -lm" \
    --enable-shared --disable-static \
    --enable-gpl --enable-version3 \
    --enable-zlib --enable-bzlib --enable-lzma \
    --enable-libopus --enable-libvorbis \
    --enable-libdav1d --enable-libopenh264 --enable-libx264 --enable-libx265 \
    --enable-libvpx --enable-libtheora --enable-libmp3lame \
    --disable-doc --disable-debug \
    --disable-vaapi --disable-vdpau --disable-xlib --disable-libxcb \
    --disable-sdl2 --disable-alsa --disable-libdrm \
    "${extra[@]}"
}
do_build()   { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
