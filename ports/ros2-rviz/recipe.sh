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
DEPS="ros2 qt6 assimp freetype zlib eigen orocos-kdl"
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
