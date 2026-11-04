# Helpers for ROS 2 overlay components: a colcon workspace of upstream ROS
# repositories built on top of the ros2 component (its staged install as the
# underlay) and installed into the same prefix, /usr/local/opt/ros/jazzy --
# the way ports/ros2-rviz builds RViz. Sourced by recipe.sh files that set:
#
#   ROS_PREFIX        "$PREFIX/opt/ros/jazzy"
#   OVERLAY_REPOS     a vcstool .repos file (repositories at pinned versions)
#   OVERLAY_PATCHES   a directory with MAP ("<patch> <dir>" lines, dir relative
#                     to the source root) and the patches it names (git apply)
#   OVERLAY_IGNORE    directories (relative to the source root) colcon skips
#   OVERLAY_CMAKE_ARGS (array) extra -D arguments for every package
#   OVERLAY_PIP       extra host-Python packages the build runs (generators)
#
# The underlay is everything already staged in the ROS prefix (the ros2
# component and any overlay this one depends on): colcon finds packages
# through their share/colcon-core/packages entries.
_ros_overlay_venv() {
  # Its own venv of the build host's Python 3.14 (the target's version, which
  # FindPython3 matches against the target headers): the ros2 component's
  # tools plus what overlay generators import.
  local v="$BUILD_DIR/host/ros2-overlay-venv-py3.14"
  if [ ! -x "$v/bin/colcon" ] || ! "$v/bin/python" -c "import jinja2, typeguard" 2>/dev/null; then
    "$WS/hosttools/bin/python3.14" -m venv "$v" && "$v/bin/pip" install -q vcstool colcon-common-extensions \
      "empy==3.3.4" lark catkin_pkg pyyaml numpy setuptools==69.5.1 rosdistro rospkg pytest jinja2 typeguard lxml || return 1
  fi
  [ -z "$OVERLAY_PIP" ] || "$v/bin/pip" install -q $OVERLAY_PIP || return 1
  echo "$v"
}
do_fetch() {
  local v; v=$(_ros_overlay_venv) || return 1
  mkdir -p "$SRCROOT/src" && "$v/bin/vcs" import --shallow --retry 3 "$SRCROOT/src" < "$OVERLAY_REPOS"
}
do_patch() {
  local p dir
  if [ -f "$OVERLAY_PATCHES/MAP" ]; then
    while read -r p dir; do
      [ -n "$p" ] && [ "${p#\#}" = "$p" ] || continue
      echo "-- patch $dir: $p"
      git -C "$SRCROOT/$dir" apply --whitespace=nowarn "$OVERLAY_PATCHES/$p" || return 1
    done < "$OVERLAY_PATCHES/MAP"
  fi
  for dir in $OVERLAY_IGNORE; do [ -d "$SRCROOT/$dir" ] && touch "$SRCROOT/$dir/COLCON_IGNORE"; done
  return 0
}
do_configure() { :; }
do_build() {
  local v; v=$(_ros_overlay_venv) || return 1
  local U="$STAGE_SYSROOT$ROS_PREFIX" L="$STAGE_SYSROOT$PREFIX/lib" pv=3.14
  # ros2's toolchain file (qnx-ports'), with what the ros2 recipe sets on top
  # of it, so that the overlay is compiled exactly like the core it links to;
  # its CMAKE_FIND_ROOT_PATH gets the underlay and the staging tree.
  cat > "$B/overlay-toolchain.cmake" <<TC || return 1
include("$RECIPE_DIR/ros2/qnx-ports/platform/qnx.nto.toolchain.cmake")
set(PYTHON_VERSION_QNX $pv)
set(PYTHON_SOABI cpython-${pv/./})
set(PYTHON_MODULE_EXTENSION .cpython-${pv/./}.so)
set(Eigen3_INCLUDE_DIRS "$STAGE_SYSROOT$PREFIX/include/eigen3")
unset(Eigen3_FOUND)
unset(EIGEN3_FOUND)
list(APPEND CMAKE_FIND_ROOT_PATH "$U" "$STAGE_SYSROOT$PREFIX" "$STAGE_SYSROOT")
list(APPEND CMAKE_PREFIX_PATH "$U" "$STAGE_SYSROOT$PREFIX")
include_directories(SYSTEM "$STAGE_SYSROOT$PREFIX/include/python$pv")
foreach(k EXE SHARED MODULE)
  if(NOT CMAKE_\${k}_LINKER_FLAGS MATCHES "no-as-needed -lc")
    set(CMAKE_\${k}_LINKER_FLAGS "-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state -L$L -Wl,-rpath-link,$L -Wl,-rpath-link,$U/lib \${CMAKE_\${k}_LINKER_FLAGS}" CACHE STRING "" FORCE)
  endif()
endforeach()
TC
  # colcon puts each installed package's lib directory on LD_LIBRARY_PATH;
  # they hold QNX libraries. Run the host's cmake and ctest without it.
  local w="$B/hostwrap" t
  mkdir -p "$w" && for t in cmake ctest; do
    printf '#!/bin/sh\nexec env -u LD_LIBRARY_PATH %s "$@"\n' "$(command -v $t)" > "$w/$t" && chmod +x "$w/$t" || return 1
  done
  cd "$S" && ( COLCON_CURRENT_PREFIX="$U" . "$U/local_setup.sh" && \
  PATH="$w:$v/bin:$PATH" CPU="$QNX_CPU" ARCH="$QNX_CPU" CPUVARDIR="$QNX_ARCH" CPUVAR="$QNX_ARCH" QNX_SDP_VERSION=qnx800 LC_NUMERIC=en_US.UTF-8 \
  colcon build "$@" --merge-install --cmake-force-configure --build-base "$B/build" --install-base "$B/install" \
    --event-handlers console_direct- --cmake-args -DCMAKE_TOOLCHAIN_FILE="$B/overlay-toolchain.cmake" \
      -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release --no-warn-unused-cli -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
      -DEIGEN3_INCLUDE_DIR="$STAGE_SYSROOT$PREFIX/include/eigen3" -DEigen3_DIR="$STAGE_SYSROOT$PREFIX/share/eigen3/cmake" \
      -DPython3_INCLUDE_DIR="$STAGE_SYSROOT$PREFIX/include/python$pv" -DPython3_LIBRARY="$L/libpython$pv.so" \
      -DPython3_NumPy_INCLUDE_DIR="$STAGE_SYSROOT$PREFIX/lib/python$pv/site-packages/numpy/_core/include" \
      -DTRACETOOLS_TRACEPOINTS_EXCLUDED=ON "${OVERLAY_CMAKE_ARGS[@]}" -DCMAKE_INSTALL_PREFIX="$ROS_PREFIX" )
}
# Into the ROS prefix, only what these packages installed: the workspace-level
# setup scripts are the ros2 component's.
do_install() {
  local x="$B/install-exclude"
  printf '%s\n' './setup.*' './local_setup.*' './_local_setup_util*' './COLCON_IGNORE' './.colcon_install_layout' > "$x"
  mkdir -p "$STAGE$ROS_PREFIX" && ( cd "$B/install" && tar cf - --anchored -X "$x" . ) | ( cd "$STAGE$ROS_PREFIX" && tar xf - )
}
