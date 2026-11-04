# ros2-rviz 14.1.24

> RViz 2 for ROS 2 Jazzy: the 3D visualiser, drawn natively on QNX Screen (OGRE on OpenGL ES)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`ros2`](../ros2/) [`qt6`](../qt6/) [`assimp`](../assimp/) [`freetype`](../freetype/) [`zlib`](../zlib/) [`eigen`](../eigen/) |
| Run-time dependencies | [`assimp`](../assimp/) [`console_bridge`](../console_bridge/) [`freetype`](../freetype/) [`orocos-kdl`](../orocos-kdl/) [`qt6`](../qt6/) [`tinyxml2`](../tinyxml2/) [`yaml-cpp`](../yaml-cpp/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_ros2_rviz`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `ros2-rviz` (programs, shared libraries, run-time data) and `ros2-rviz-dev` (headers, pkg-config/CMake files, static libraries; needs `ros2-rviz`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`ros2-rviz`](https://openqnx.github.io/oss/package/x86_64/ros2-rviz.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ros2-rviz.html)) · [`ros2-rviz-dev`](https://openqnx.github.io/oss/package/x86_64/ros2-rviz-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ros2-rviz-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add ros2-rviz` (dependencies come along); one version: `apk add ros2-rviz=<version>` | `apk del ros2-rviz` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install ros2-rviz` (dependencies first) | `oss-get remove ros2-rviz` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`ros2-rviz-dev` the same way, by its own name (`apk add ros2-rviz-dev`, `oss-get install ros2-rviz-dev`, by hand with its own file and list `ros2-rviz-dev.list`); remove it before `ros2-rviz`, which it needs.

```sh
# on the QNX target, as root
apk add ros2-rviz ros2-rviz-dev
apk add qnx-test-ros2-rviz && /usr/local/bin/qnx-port-tests/test_ros2_rviz    # its on-target test, from tests-8.0-<arch>-m-z
apk del ros2-rviz ros2-rviz-dev qnx-test-ros2-rviz
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/ros2-rviz.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/ros2-rviz.list
rm var/lib/manual/ros2-rviz.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ros2-rviz` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (84 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild ros2 qt6 assimp freetype zlib eigen
```

<details><summary>Full build chain (84)</summary>

[`aom`](../aom/) [`assimp`](../assimp/) [`benchmark`](../benchmark/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`cairo`](../cairo/) [`console_bridge`](../console_bridge/) [`curl`](../curl/) [`cyclonedds`](../cyclonedds/) [`dav1d`](../dav1d/) [`double-conversion`](../double-conversion/) [`eigen`](../eigen/) [`expat`](../expat/) [`fastcdr`](../fastcdr/) [`fastdds`](../fastdds/) [`fdk-aac`](../fdk-aac/) [`ffmpeg`](../ffmpeg/) [`fontconfig`](../fontconfig/) [`foonathan_memory`](../foonathan_memory/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gst-plugins-bad`](../gst-plugins-bad/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`lame`](../lame/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libtheora`](../libtheora/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libvpx`](../libvpx/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxft`](../libxft/) [`libxml2`](../libxml2/) [`libxrender`](../libxrender/) [`libyaml`](../libyaml/) [`lz4`](../lz4/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`numpy`](../numpy/) [`openblas`](../openblas/) [`opencv`](../opencv/) [`openh264`](../openh264/) [`openssl`](../openssl/) [`opus`](../opus/) [`orc`](../orc/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`qt6`](../qt6/) [`readline`](../readline/) [`ros2`](../ros2/) [`sqlite`](../sqlite/) [`svt-av1`](../svt-av1/) [`tinyxml2`](../tinyxml2/) [`x264`](../x264/) [`x265`](../x265/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`yaml-cpp`](../yaml-cpp/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

```bash
do_fetch() {
  local v; v=$(_rosvenv) || return 1
  mkdir -p "$SRCROOT/src" && "$v/bin/vcs" import --shallow --retry 3 "$SRCROOT/src" < "$RECIPE_DIR/ros2-rviz/rviz.repos"
}
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

More QNX changes, outside the main source tree:

- [`repo-patches/`](repo-patches/) — 3 one patch per repository of a multi-repository source (repo-patches/MAP)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/ros2-rviz; B=build/$QNX_ARCH/ros2-rviz; mkdir -p $B
```

```bash
do_patch() {
  local p dir
  while read -r p dir; do
    echo "-- patch $dir: $p"
    git -C "$SRCROOT/$dir" apply --whitespace=nowarn "$RECIPE_DIR/ros2-rviz/repo-patches/$p" || return 1
  done < "$RECIPE_DIR/ros2-rviz/repo-patches/MAP"
  # OGRE's own QNX changes go with rviz_ogre_vendor's patches, which
  # ament_vendor applies to the OGRE tree it clones.
  cp "$RECIPE_DIR/ros2-rviz/ogre-patches/"*.patch "$SRCROOT/src/ros2/rviz/rviz_ogre_vendor/patches/" || return 1
  # Test and visual-test packages are not built (BUILD_TESTING is off; they
  # want gtest/gmock fixtures and an X display).
  touch "$SRCROOT/src/ros2/rviz/rviz_rendering_tests/COLCON_IGNORE" "$SRCROOT/src/ros2/rviz/rviz_visual_testing_framework/COLCON_IGNORE"
}
```

```bash
do_configure() { :; }
```

```bash
do_build() {
  local v; v=$(_rosvenv) || return 1
  local U="$STAGE_SYSROOT$ROS_PREFIX" L="$STAGE_SYSROOT$PREFIX/lib" pv=3.14
  # ros2's toolchain file (qnx-ports'), with what the ros2 recipe sets on top
  # of it, so that the overlay is compiled exactly like the core it links to.
  # Its CMAKE_FIND_ROOT_PATH is the install prefix and the SDP only: add the
  # underlay (the staged ros2 install) and the staging tree, for Qt 6,
  # assimp, freetype and eigen.
  cat > "$B/rviz-toolchain.cmake" <<EOF || return 1
include("$RECIPE_DIR/ros2/qnx-ports/platform/qnx.nto.toolchain.cmake")
set(PYTHON_VERSION_QNX $pv)
set(PYTHON_SOABI cpython-${pv/./})
set(Eigen3_INCLUDE_DIRS "$STAGE_SYSROOT$PREFIX/include/eigen3")
# It also claims Eigen3_FOUND, and ament's exported-dependency hooks skip a
# find_package() of what is already "found": rviz_common then had no
# Eigen3::Eigen target for rviz_rendering's interface ("the target was not
# found"). Let Eigen's own config find it.
unset(Eigen3_FOUND)
unset(EIGEN3_FOUND)
list(APPEND CMAKE_FIND_ROOT_PATH "$U" "$STAGE_SYSROOT$PREFIX" "$STAGE_SYSROOT")
list(APPEND CMAKE_PREFIX_PATH "$U" "$STAGE_SYSROOT$PREFIX")
# Same as ros2: keep libc needed under the toolchain's --as-needed.
foreach(k EXE SHARED MODULE)
  if(NOT CMAKE_\${k}_LINKER_FLAGS MATCHES "no-as-needed -lc")
    set(CMAKE_\${k}_LINKER_FLAGS "-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state -L$L -Wl,-rpath-link,$L -Wl,-rpath-link,$U/lib -Wl,-rpath-link,$U/opt/rviz_ogre_vendor/lib \${CMAKE_\${k}_LINKER_FLAGS}" CACHE STRING "" FORCE)
  endif()
endforeach()
EOF
  # colcon puts each installed package's lib directory on LD_LIBRARY_PATH;
  # they hold QNX libraries. Run the host's cmake and ctest without it (as
  # the ros2 recipe does).
  local w="$B/hostwrap" t
  mkdir -p "$w" && for t in cmake ctest; do
    printf '#!/bin/sh\nexec env -u LD_LIBRARY_PATH %s "$@"\n' "$(command -v $t)" > "$w/$t" && chmod +x "$w/$t" || return 1
  done
  # The underlay: the staged ros2 install, sourced from where it is staged
  # (COLCON_CURRENT_PREFIX) so ament finds its packages, and the host Python
  # its pure-Python build tools (ament_package, ...).
  cd "$S" && ( COLCON_CURRENT_PREFIX="$U" . "$U/local_setup.sh" && \
  PATH="$w:$v/bin:$PATH" CPU="$QNX_CPU" ARCH="$QNX_CPU" CPUVARDIR="$QNX_ARCH" CPUVAR="$QNX_ARCH" QNX_SDP_VERSION=qnx800 LC_NUMERIC=en_US.UTF-8 \
  colcon build "$@" --merge-install --cmake-force-configure --build-base "$B/build" --install-base "$B/install" \
    --event-handlers console_direct- --cmake-args -DCMAKE_TOOLCHAIN_FILE="$B/rviz-toolchain.cmake" \
      -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release --no-warn-unused-cli \
      -DEIGEN3_INCLUDE_DIR="$STAGE_SYSROOT$PREFIX/include/eigen3" -DEigen3_DIR="$STAGE_SYSROOT$PREFIX/share/eigen3/cmake" \
      -DQT_HOST_PATH="$WS/hosttools/qt6-6.8.1" -DQt6_DIR="$L/cmake/Qt6" \
      -Dassimp_DIR="$(ls -d "$L"/cmake/assimp-* 2>/dev/null | head -1)" \
      -DCMAKE_INSTALL_PREFIX="$ROS_PREFIX" )
}
```

```bash
do_install() {
  local og="$B/build/rviz_ogre_vendor/ogre_vendor-prefix/install" x="$B/install-exclude"
  ( cd "$og" && find . \( -type f -o -type l \) ) > "$x" || return 1
  printf '%s\n' './setup.*' './local_setup.*' './_local_setup_util*' './COLCON_IGNORE' './.colcon_install_layout' >> "$x"
  mkdir -p "$STAGE$ROS_PREFIX" && ( cd "$B/install" && tar cf - --anchored -X "$x" . ) | ( cd "$STAGE$ROS_PREFIX" && tar xf - ) || return 1
  # (tar keeps the directories of what it left out; STAGE is this package's own root here)
  local d; for d in include/OGRE lib/OGRE lib/pkgconfig share/OGRE; do
    [ -d "$STAGE$ROS_PREFIX/$d" ] && find "$STAGE$ROS_PREFIX/$d" -depth -type d -empty -delete
  done; return 0
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- RViz 2 (ROS 2 Jazzy's 3D visualiser) drawing natively on QNX Screen, with interactive_markers, which it needs -- built as an overlay workspace on top of the ros2 component and installed into the same prefix (/usr/local/opt/ros/jazzy: source its setup.sh, then run rviz2). Upstream sources at the tags in rviz.repos (RViz 14.1.24, the latest Jazzy release; interactive_markers 2.5.5); QNX changes in repo-patches/ (MAP says which repository each one is for). OGRE 1.12.10, the version RViz Jazzy pins, is built by rviz_ogre_vendor (it clones the v1.12.10 tag at build time). QNX has no desktop OpenGL and no GLX: only OpenGL ES 2/3 through EGL on Screen. OGRE 1.12 has an ES 2/3 render system (RenderSystem_GLES2) over EGL for Android, Emscripten and X11 but none for QNX; ogre-patches/ adds one (an EGL window on a screen_window_t, created by OGRE or handed over by Qt), and RViz is patched to load it and to emulate the fixed-function pipeline ES lacks with OGRE's RTShader System. Qt: this tree has Qt 6 (ports/qt6); Jazzy's RViz is Qt 5 only, so upstream's Qt 6 support from rolling is backported (repo-patches/rviz-qt6.patch).
- ros2: the ROS libraries and setup.sh it runs on (not found from NEEDED: they are in the ROS prefix, not /usr/local/lib).
- It also claims Eigen3_FOUND, and ament's exported-dependency hooks skip a find_package() of what is already "found": rviz_common then had no Eigen3::Eigen target for rviz_rendering's interface ("the target was not found"). Let Eigen's own config find it.
- Same as ros2: keep libc needed under the toolchain's --as-needed.
- Into the ros2 prefix, but only what these packages installed: the workspace-level setup scripts are the ros2 component's (colcon finds the overlay's packages through their own share/colcon-core/packages entries). The ros2 component's ament_vendor (qnx-ports' ament_cmake change) installs a vendored project twice, to opt/<vendor>/ and to the prefix itself: OGRE's libraries would ship as two copies of each soname. Keep the opt/ one, which RViz loads its plugins from and rviz_ogre_vendor's environment hook puts on LD_LIBRARY_PATH.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ros2-rviz`.

### 5. What it installs

1098 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/ros2-rviz` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ros2-rviz
ls dists/8.0/*/ | grep '^ros2-rviz-'
```

This writes `ros2-rviz-<ver>-r0.tar.gz` (run time) and `ros2-rviz-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ros2-rviz
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ros2-rviz-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ros2-rviz-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_ros2_rviz`](../../tests/src/) links ros2-rviz the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-ros2-rviz`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-ros2-rviz
/usr/local/bin/qnx-port-tests/test_ros2_rviz
```

```
== ros2-rviz ==
  ok   ...
PASS ros2-rviz
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_ros2_rviz` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_ros2_rviz`](../../tests/src/) -- a program linked against ros2-rviz the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> ros2-rviz`.

## Versions

This recipe builds **14.1.24**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ros2-rviz=<version>`), sources in `ported/ros2-rviz/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ros2-rviz`; port one with `scripts/port-update ros2-rviz`.

<details><summary>recipe.sh</summary>

```bash
VERSION=14.1.24
# RViz 2 (ROS 2 Jazzy's 3D visualiser) drawing natively on QNX Screen, with
# interactive_markers, which it needs -- built as an overlay workspace on top
# of the ros2 component and installed into the same prefix
# (/usr/local/opt/ros/jazzy: source its setup.sh, then run rviz2).
#
# Upstream sources at the tags in rviz.repos (RViz 14.1.24, the latest Jazzy
# release; interactive_markers 2.5.5); QNX changes in repo-patches/ (MAP says
# which repository each one is for).
#
# OGRE 1.12.10, the version RViz Jazzy pins, is built by rviz_ogre_vendor
# (it clones the v1.12.10 tag at build time). QNX has no desktop OpenGL and no
# GLX: only OpenGL ES 2/3 through EGL on Screen. OGRE 1.12 has an ES 2/3
# render system (RenderSystem_GLES2) over EGL for Android, Emscripten and X11
# but none for QNX; ogre-patches/ adds one (an EGL window on a
# screen_window_t, created by OGRE or handed over by Qt), and RViz is
# patched to load it and to emulate the fixed-function pipeline ES lacks
# with OGRE's RTShader System.
#
# Qt: this tree has Qt 6 (ports/qt6); Jazzy's RViz is Qt 5 only, so upstream's
# Qt 6 support from rolling is backported (repo-patches/rviz-qt6.patch).
DEPS="ros2 qt6 assimp freetype zlib eigen"
# ros2: the ROS libraries and setup.sh it runs on (not found from NEEDED: they
# are in the ROS prefix, not /usr/local/lib).
RUNTIME_DEPS="ros2"
BUILDSYS=custom
ROS_PREFIX="$PREFIX/opt/ros/jazzy"
_rosvenv() {
  # The same host venv the ros2 component builds with (colcon, vcstool,
  # catkin_pkg, empy for ament).
  local v="$BUILD_DIR/host/ros2-venv-py3.14"
  [ -x "$v/bin/colcon" ] || { "$WS/hosttools/bin/python3.14" -m venv "$v" && "$v/bin/pip" install -q vcstool colcon-common-extensions \
      "empy==3.3.4" lark catkin_pkg pyyaml numpy setuptools==69.5.1 rosdistro rospkg pytest; } || return 1
  echo "$v"
}
do_fetch() {
  local v; v=$(_rosvenv) || return 1
  mkdir -p "$SRCROOT/src" && "$v/bin/vcs" import --shallow --retry 3 "$SRCROOT/src" < "$RECIPE_DIR/ros2-rviz/rviz.repos"
}
do_patch() {
  local p dir
  while read -r p dir; do
    echo "-- patch $dir: $p"
    git -C "$SRCROOT/$dir" apply --whitespace=nowarn "$RECIPE_DIR/ros2-rviz/repo-patches/$p" || return 1
  done < "$RECIPE_DIR/ros2-rviz/repo-patches/MAP"
  # OGRE's own QNX changes go with rviz_ogre_vendor's patches, which
  # ament_vendor applies to the OGRE tree it clones.
  cp "$RECIPE_DIR/ros2-rviz/ogre-patches/"*.patch "$SRCROOT/src/ros2/rviz/rviz_ogre_vendor/patches/" || return 1
  # Test and visual-test packages are not built (BUILD_TESTING is off; they
  # want gtest/gmock fixtures and an X display).
  touch "$SRCROOT/src/ros2/rviz/rviz_rendering_tests/COLCON_IGNORE" "$SRCROOT/src/ros2/rviz/rviz_visual_testing_framework/COLCON_IGNORE"
}
do_configure() { :; }
do_build() {
  local v; v=$(_rosvenv) || return 1
  local U="$STAGE_SYSROOT$ROS_PREFIX" L="$STAGE_SYSROOT$PREFIX/lib" pv=3.14
  # ros2's toolchain file (qnx-ports'), with what the ros2 recipe sets on top
  # of it, so that the overlay is compiled exactly like the core it links to.
  # Its CMAKE_FIND_ROOT_PATH is the install prefix and the SDP only: add the
  # underlay (the staged ros2 install) and the staging tree, for Qt 6,
  # assimp, freetype and eigen.
  cat > "$B/rviz-toolchain.cmake" <<EOF || return 1
include("$RECIPE_DIR/ros2/qnx-ports/platform/qnx.nto.toolchain.cmake")
set(PYTHON_VERSION_QNX $pv)
set(PYTHON_SOABI cpython-${pv/./})
set(Eigen3_INCLUDE_DIRS "$STAGE_SYSROOT$PREFIX/include/eigen3")
# It also claims Eigen3_FOUND, and ament's exported-dependency hooks skip a
# find_package() of what is already "found": rviz_common then had no
# Eigen3::Eigen target for rviz_rendering's interface ("the target was not
# found"). Let Eigen's own config find it.
unset(Eigen3_FOUND)
unset(EIGEN3_FOUND)
list(APPEND CMAKE_FIND_ROOT_PATH "$U" "$STAGE_SYSROOT$PREFIX" "$STAGE_SYSROOT")
list(APPEND CMAKE_PREFIX_PATH "$U" "$STAGE_SYSROOT$PREFIX")
# Same as ros2: keep libc needed under the toolchain's --as-needed.
foreach(k EXE SHARED MODULE)
  if(NOT CMAKE_\${k}_LINKER_FLAGS MATCHES "no-as-needed -lc")
    set(CMAKE_\${k}_LINKER_FLAGS "-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state -L$L -Wl,-rpath-link,$L -Wl,-rpath-link,$U/lib -Wl,-rpath-link,$U/opt/rviz_ogre_vendor/lib \${CMAKE_\${k}_LINKER_FLAGS}" CACHE STRING "" FORCE)
  endif()
endforeach()
EOF
  # colcon puts each installed package's lib directory on LD_LIBRARY_PATH;
  # they hold QNX libraries. Run the host's cmake and ctest without it (as
  # the ros2 recipe does).
  local w="$B/hostwrap" t
  mkdir -p "$w" && for t in cmake ctest; do
    printf '#!/bin/sh\nexec env -u LD_LIBRARY_PATH %s "$@"\n' "$(command -v $t)" > "$w/$t" && chmod +x "$w/$t" || return 1
  done
  # The underlay: the staged ros2 install, sourced from where it is staged
  # (COLCON_CURRENT_PREFIX) so ament finds its packages, and the host Python
  # its pure-Python build tools (ament_package, ...).
  cd "$S" && ( COLCON_CURRENT_PREFIX="$U" . "$U/local_setup.sh" && \
  PATH="$w:$v/bin:$PATH" CPU="$QNX_CPU" ARCH="$QNX_CPU" CPUVARDIR="$QNX_ARCH" CPUVAR="$QNX_ARCH" QNX_SDP_VERSION=qnx800 LC_NUMERIC=en_US.UTF-8 \
  colcon build "$@" --merge-install --cmake-force-configure --build-base "$B/build" --install-base "$B/install" \
    --event-handlers console_direct- --cmake-args -DCMAKE_TOOLCHAIN_FILE="$B/rviz-toolchain.cmake" \
      -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release --no-warn-unused-cli \
      -DEIGEN3_INCLUDE_DIR="$STAGE_SYSROOT$PREFIX/include/eigen3" -DEigen3_DIR="$STAGE_SYSROOT$PREFIX/share/eigen3/cmake" \
      -DQT_HOST_PATH="$WS/hosttools/qt6-6.8.1" -DQt6_DIR="$L/cmake/Qt6" \
      -Dassimp_DIR="$(ls -d "$L"/cmake/assimp-* 2>/dev/null | head -1)" \
      -DCMAKE_INSTALL_PREFIX="$ROS_PREFIX" )
}
# Into the ros2 prefix, but only what these packages installed: the
# workspace-level setup scripts are the ros2 component's (colcon finds the
# overlay's packages through their own share/colcon-core/packages entries).
# The ros2 component's ament_vendor (qnx-ports' ament_cmake change) installs a
# vendored project twice, to opt/<vendor>/ and to the prefix itself: OGRE's
# libraries would ship as two copies of each soname. Keep the opt/ one, which
# RViz loads its plugins from and rviz_ogre_vendor's environment hook puts on
# LD_LIBRARY_PATH.
do_install() {
  local og="$B/build/rviz_ogre_vendor/ogre_vendor-prefix/install" x="$B/install-exclude"
  ( cd "$og" && find . \( -type f -o -type l \) ) > "$x" || return 1
  printf '%s\n' './setup.*' './local_setup.*' './_local_setup_util*' './COLCON_IGNORE' './.colcon_install_layout' >> "$x"
  mkdir -p "$STAGE$ROS_PREFIX" && ( cd "$B/install" && tar cf - --anchored -X "$x" . ) | ( cd "$STAGE$ROS_PREFIX" && tar xf - ) || return 1
  # (tar keeps the directories of what it left out; STAGE is this package's own root here)
  local d; for d in include/OGRE lib/OGRE lib/pkgconfig share/OGRE; do
    [ -d "$STAGE$ROS_PREFIX/$d" ] && find "$STAGE$ROS_PREFIX/$d" -depth -type d -empty -delete
  done; return 0
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
