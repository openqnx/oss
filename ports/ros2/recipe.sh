VERSION=jazzy
# ROS 2 Jazzy Jalisco: the core (rclcpp, rclpy, Fast DDS and Cyclone DDS RMWs,
# rosbag2, tf2, common interfaces, demos) -- 107 repositories at the versions
# pinned in ros2.repos.
#
# Taken from qnx-ports/build-files ports/ros2/ros2-jazzy (see qnx-ports/LICENSE):
#   ros2.repos          the repository set and versions
#   ros2-extra.repos    vision_opencv (cv_bridge), imported on top of it
#   repo-patches/*.patch QNX changes, one per repository (repo-patches/MAP says where)
#   COLCON_IGNORE.list  packages that are not built for QNX (rviz, gazebo, ...)
#   qnx-ports/platform  the CMake toolchain file the ROS build uses
#   qnx-ports/modules   find modules and vendor package overrides
#
# The ROS build tools (vcstool, colcon, the rosidl generator dependencies)
# run on the build host, in a venv of the host Python.
# Installs to /usr/local/opt/ros/jazzy on the target (source its
# setup.sh/local_setup.sh there).
#
# Built against this tree's Fast DDS, Fast CDR, Cyclone DDS, console_bridge,
# libyaml, yaml-cpp, zstd, Google Benchmark and tinyxml2 rather than copies
# of its own (the Fast-CDR/Fast-DDS/cyclonedds repositories are
# in COLCON_IGNORE.list; the *_vendor packages find the components through
# the <Pkg>_DIR arguments below). The copies carried the components' sonames
# into the ROS prefix, and QNX's loader takes whichever comes first on
# LD_LIBRARY_PATH: with /usr/local/lib first, Fast DDS nodes loaded the
# standalone libfastcdr and threw BadParamException registering their first
# type. One copy of each library now, whatever the search order -- except
# Orocos KDL: ROS's PyKDL needs a later commit than the orocos-kdl component's
# 1.5.1 (std::hash for its types), so orocos_kdl_vendor still builds its own
# liborocos-kdl.so.1.5; keep /usr/local/opt/ros/jazzy/lib first for ROS (its
# setup.sh does).
DEPS="python3 numpy opencv curl lz4 fastcdr fastdds cyclonedds console_bridge libyaml yaml-cpp zstd benchmark tinyxml2"
BUILDSYS=custom
ROS_PREFIX="$PREFIX/opt/ros/$VERSION"
_rosvenv() {
  # The build host's Python of the target's version (3.14, hosttools): FindPython3
  # matches the interpreter against the target Python3_LIBRARY/INCLUDE_DIR, and
  # with the system's 3.12 it rejected them and took the SDP's 3.11 instead.
  local v="$BUILD_DIR/host/ros2-venv-py3.14"
  [ -x "$v/bin/colcon" ] || { "$WS/hosttools/bin/python3.14" -m venv "$v" && "$v/bin/pip" install -q vcstool colcon-common-extensions \
      "empy==3.3.4" lark catkin_pkg pyyaml numpy setuptools==69.5.1 rosdistro rospkg pytest; } || return 1
  echo "$v"
}
do_fetch() {
  local v; v=$(_rosvenv) || return 1
  mkdir -p "$SRCROOT/src" && "$v/bin/vcs" import --shallow --retry 3 "$SRCROOT/src" < "$RECIPE_DIR/ros2/ros2.repos" \
    && "$v/bin/vcs" import --shallow --retry 3 "$SRCROOT/src" < "$RECIPE_DIR/ros2/ros2-extra.repos"
}
do_patch() {
  local p dir
  while read -r p dir; do
    echo "-- patch $dir: $p"
    git -C "$SRCROOT/$dir" apply --whitespace=nowarn "$RECIPE_DIR/ros2/repo-patches/$p" || return 1
  done < "$RECIPE_DIR/ros2/repo-patches/MAP"
  while read -r dir; do [ -d "$SRCROOT/$dir" ] && touch "$SRCROOT/$dir/COLCON_IGNORE"; done < "$RECIPE_DIR/ros2/COLCON_IGNORE.list"
  return 0
}
do_configure() { :; }
# The qnx-ports toolchain file and patches read CPU, ARCH, CPUVARDIR and
# QNX_SDP_VERSION (qnx800; zstd_vendor's CMakeLists tests it) from the
# environment (ARCH names the archiver, nto<ARCH>-ar: without it "Error running
# link command" on the first static library).
# Eigen3_INCLUDE_DIRS: qnx-ports' orocos_kdl_vendor change forwards it to the
# orocos_kdl sub-build as EIGEN3_INCLUDE_DIR; unset, that was empty ("Eigen/Core:
# No such file or directory").
# OpenCV_DIR: that toolchain's CMAKE_FIND_ROOT_PATH is the colcon install
# prefix and the SDP only, so the opencv component's config is out of reach
# (image_geometry, cv_bridge: "Could not find a package configuration file
# provided by OpenCV"). Name it alone rather than add the whole staging tree,
# which would let the vendor packages settle for staged copies of what they
# vendor.
# Boost likewise (cv_bridge: "Could NOT find Boost"), as headers through
# FindBoost's own search: BoostConfig.cmake finds its components by absolute
# path, which that find-root setup reroots out of reach ("Could not find a
# package configuration file provided by boost_headers"). cv_bridge is built
# without its Python bindings (CV_BRIDGE_DISABLE_PYTHON): they need
# Boost.Python, which the boost component leaves out.
# Python3_*: the target's Python is this tree's python3 (3.14), not the SDP's
# libpython3.11, whose headers the SDP does not ship ("Python3::Python
# includes non-existent path .../python3.11").
do_build() {
  local v; v=$(_rosvenv) || return 1
  # qnx-ports' FindPython3.cmake names the SDP's Python 3.11 (headers and
  # libpython3.11 under QNX_TARGET); this tree's target Python is the python3
  # component, 3.14. Use a copy of their module directory with that one
  # module pointed at it, and at the numpy component's headers.
  local m="$B/cmake-modules" pv=3.14
  rm -rf "$m" && cp -a "$RECIPE_DIR/ros2/qnx-ports/modules" "$m" && \
  sed -i -e "s|^set(Python3_INCLUDE_DIR .*|set(Python3_INCLUDE_DIR \"$STAGE$PREFIX/include/python$pv\")|" \
         -e "s|^set(Python3_LIBRARY .*|set(Python3_LIBRARY \"$STAGE$PREFIX/lib/libpython$pv.so\")|" \
         -e "s|^set(Python3_VERSION .*|set(Python3_VERSION \"$pv\")|" \
         -e "/^set(NumPy_INCLUDE_DIR /,/)\$/c\\set(NumPy_INCLUDE_DIR \"$STAGE$PREFIX/lib/python$pv/site-packages/numpy/_core/include\")" \
         "$m/FindPython3.cmake" || return 1
  # qnx-ports' toolchain file knows only the SDP's Python 3.11 or 3.8 -- SDP 8
  # ships neither, so it settled on 3.8 and would name extension modules
  # .cpython-38.so -- and points Eigen3_INCLUDE_DIRS at the colcon install
  # prefix, where their eigen3_vendor would have put it ("Eigen/Core: No such
  # file or directory" in orocos_kdl). Include it from a wrapper that sets
  # both again for this tree's python3 and eigen components. Their patches to
  # rclpy, tf2_py, tf2_geometry_msgs, rosbag2_py and python_orocos_kdl_vendor
  # name the Python headers as ${QNX_TARGET}/usr/include/python<ver>; put the
  # staged ones on every target's include path instead ("Python.h: No such
  # file or directory").
  local inc="$STAGE$PREFIX/include/python$pv" lib="$STAGE$PREFIX/lib/libpython$pv.so"
  cat > "$B/ros2-toolchain.cmake" <<EOF || return 1
include("$RECIPE_DIR/ros2/qnx-ports/platform/qnx.nto.toolchain.cmake")
set(PYTHON_VERSION_QNX $pv)
set(PYTHON_VERSION_QNX_ABI ${pv/./})
set(PYTHON_SOABI cpython-${pv/./})
set(PYTHON_MODULE_EXTENSION .cpython-${pv/./}.so)
set(PYTHON_INCLUDE_DIR "$inc;$STAGE$PREFIX/lib/python$pv/site-packages/numpy/_core/include")
set(PYTHON_INCLUDE_DIRS "$inc;$STAGE$PREFIX/lib/python$pv/site-packages/numpy/_core/include")
set(PYTHON_LIBRARY "$lib")
set(PYTHON_LIBRARIES "$lib")
set(NUMPY_INSTALL_DIR "$STAGE$PREFIX/lib/python$pv/site-packages")
set(Eigen3_INCLUDE_DIRS "$STAGE$PREFIX/include/eigen3")
include_directories(SYSTEM \${PYTHON_INCLUDE_DIR})
# Their linker flags carry --as-needed: a library that calls nothing in libc
# itself (rcl_logging_noop, the generated *__rosidl_typesupport_c libraries)
# then lost libc.so and exported copies of libc functions from QNX's static
# libcS.a, which every other ROS library then called instead of libc's. Keep
# libc needed (toolchain/qnx-x86_64.cmake does the same for every other port).
foreach(k EXE SHARED MODULE)
  if(NOT CMAKE_\${k}_LINKER_FLAGS MATCHES "no-as-needed -lc")
    set(CMAKE_\${k}_LINKER_FLAGS "-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state \${CMAKE_\${k}_LINKER_FLAGS}" CACHE STRING "" FORCE)
  endif()
endforeach()
EOF
  # qnx-ports' geometry2 change has tf2_bullet depend on their bullet3_vendor
  # (Bullet 2.89, cloned and built at build time) where upstream wants a system
  # Bullet; put it in the workspace. Their other vendor packages stand in for
  # eigen, opencv and numpy, which come from this tree's components instead.
  rm -rf "$S/src/qnx-vendor" && mkdir -p "$S/src/qnx-vendor" && \
    cp -a "$RECIPE_DIR/ros2/qnx-ports/modules/bullet3_vendor" "$S/src/qnx-vendor/" || return 1
  # It hands Bullet's own build -DCMAKE_C/CXX_FLAGS="-Wno-ignored-attributes",
  # which replaces the toolchain's flags -- -Vgcc_nto<cpu> with them -- and
  # qcc then compiles for its default target: the aarch64le build shipped
  # x86-64 Bullet libraries (scripts/check-artifacts). Add to them instead.
  sed -i -e 's|-DCMAKE_CXX_FLAGS="-Wno-ignored-attributes"|"-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -Wno-ignored-attributes"|' \
         -e 's|-DCMAKE_C_FLAGS="-Wno-ignored-attributes"|"-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} -Wno-ignored-attributes"|' \
    "$S/src/qnx-vendor/bullet3_vendor/CMakeLists.txt" || return 1
  # colcon puts each installed package's lib directory on LD_LIBRARY_PATH.
  # Those hold QNX libraries, and on an x86_64 build host the loader accepts
  # them: once zstd_vendor was installed, the host cmake loaded its QNX
  # libzstd.so.1 and died on that library's libregex.so.1 ("cmake: error while
  # loading shared libraries"). Run the host's cmake and ctest without it.
  local w="$B/hostwrap" t
  mkdir -p "$w" && for t in cmake ctest; do
    printf '#!/bin/sh\nexec env -u LD_LIBRARY_PATH %s "$@"\n' "$(command -v $t)" > "$w/$t" && chmod +x "$w/$t" || return 1
  done
  local L="$STAGE$PREFIX/lib"
  cd "$S" && PATH="$w:$v/bin:$PATH" CPU="$QNX_CPU" ARCH="$QNX_CPU" CPUVARDIR="$QNX_ARCH" CPUVAR="$QNX_ARCH" QNX_SDP_VERSION=qnx800 LC_NUMERIC=en_US.UTF-8 \
  colcon build --merge-install --cmake-force-configure --build-base "$B/build" --install-base "$B/install" \
    --cmake-args -DCMAKE_TOOLCHAIN_FILE="$B/ros2-toolchain.cmake" \
      -DCMAKE_MODULE_PATH="$B/cmake-modules" -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release \
      -DTHIRDPARTY=FORCE -DCPU="$QNX_CPU" -DSECURITY=ON --no-warn-unused-cli \
      -DEIGEN3_INCLUDE_DIR="$STAGE$PREFIX/include/eigen3" -DEigen3_INCLUDE_DIRS="$STAGE$PREFIX/include/eigen3" -DOpenCV_DIR="$STAGE$PREFIX/lib/cmake/opencv4" -DBoost_NO_BOOST_CMAKE=ON -DBoost_INCLUDE_DIR="$STAGE$PREFIX/include" -DCV_BRIDGE_DISABLE_PYTHON=ON -DCMAKE_INSTALL_PREFIX="$ROS_PREFIX" \
      -DPython3_INCLUDE_DIR="$STAGE$PREFIX/include/python3.14" -DPython3_LIBRARY="$STAGE$PREFIX/lib/libpython3.14.so" \
      -DPython3_NumPy_INCLUDE_DIR="$STAGE$PREFIX/lib/python3.14/site-packages/numpy/_core/include" \
      -Dfastcdr_DIR="$L/cmake/fastcdr" -Dfastrtps_DIR="$STAGE$PREFIX/share/fastrtps/cmake" \
      -Dfoonathan_memory_DIR="$L/foonathan_memory/cmake" -DTinyXML2_DIR="$L/cmake/tinyxml2" -Dtinyxml2_DIR="$L/cmake/tinyxml2" \
      -DCycloneDDS_DIR="$L/cmake/CycloneDDS" -Dconsole_bridge_DIR="$L/console_bridge/cmake" \
      -Dyaml_DIR="$L/cmake/yaml" -Dyaml-cpp_DIR="$L/cmake/yaml-cpp" \
      -Dbenchmark_DIR="$L/cmake/benchmark" -Dzstd_INCLUDE_DIR="$STAGE$PREFIX/include" -Dzstd_LIBRARY="$L/libzstd.so" \
      -DFastRTPS_INCLUDE_DIR="$STAGE$PREFIX/include" -DFastRTPS_LIBRARY_RELEASE="$L/libfastrtps.so" -DFastCDR_LIBRARY_RELEASE="$L/libfastcdr.so"
}
do_install() { mkdir -p "$STAGE$ROS_PREFIX" && cp -a "$B/install/." "$STAGE$ROS_PREFIX/"; }
