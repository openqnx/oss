VERSION=2.16.1; GIT=https://github.com/tensorflow/tensorflow.git; REF=v2.16.1
# TensorFlow Lite (the C/C++ inference runtime, tensorflow/lite), the release
# qnx-ports ports. patches/ holds TensorFlow's own QNX changes.
#
# TFLite's CMake downloads its dependencies at configure time (FetchContent),
# pinned to exact commits. Their QNX changes come in two kinds:
#  - deps/fetchcontent/{pthreadpool,XNNPACK}.patch: applied by
#    TensorFlow's own download step (the QNX patch adds PATCH_COMMANDs that read
#    them from QNX_PATCH_DIR) -- from qnx-ports/build-files.
#  - deps/<name>/*.patch for abseil-cpp, cpuinfo and farmhash, which
#    qnx-ports replaces with its forks: here the dependency is fetched at the
#    commit TensorFlow pins, these patches (the forks' QNX commits) are applied,
#    and FetchContent is pointed at that tree. cpuinfo is the exception: its
#    fork's QNX commits are based on 3c8b1533 (April 2024), newer than the
#    commit TensorFlow pins, and do not apply to that one -- so it is fetched
#    at the fork's base, as ports/cpuinfo is. ruy builds unchanged (the fork's
#    only change links its tests with libregex).
BUILDSYS=custom
SUBDIR=tensorflow/lite
# name|repository|commit TensorFlow 2.16.1 pins (tensorflow/lite/tools/cmake/modules),
# except cpuinfo (see above). ml_dtypes is fetched here too, without its git
# submodules: its third_party/eigen is a gitlab.com submodule (see the Eigen
# note below), and TFLite only uses ml_dtypes' headers, with its own Eigen.
TFL_DEPS="abseil-cpp|https://github.com/abseil/abseil-cpp|fb3621f4f897824c0dbe0615fa94543df6192f30
cpuinfo|https://github.com/pytorch/cpuinfo|3c8b1533ac03dd6531ab6e7b9245d488f13a82a5
ruy|https://github.com/google/ruy|3286a34cc8de6149ac6844107dfdffac91531e72
farmhash|https://github.com/google/farmhash|0d859a811870d10f53a594927d0d0b97573ad06d
ml_dtypes|https://github.com/jax-ml/ml_dtypes|780b6d0ee01ffbfac45f7ec5418bc08f2b166483"

do_patch() {
  local line name url commit dir p
  for line in $TFL_DEPS; do
    IFS='|' read -r name url commit <<<"$line"
    dir="$SRCROOT/_qnx_deps/$name"
    git init -q "$dir" && git -C "$dir" remote add origin "$url" && \
      git -C "$dir" fetch -q --depth 1 origin "$commit" && \
      git -C "$dir" -c advice.detachedHead=false checkout -q FETCH_HEAD || return 1
    for p in "$RECIPE_DIR/tensorflow-lite/deps/$name"/*.patch; do
      [ -e "$p" ] || continue
      echo "-- patch $name: $(basename "$p")"
      patch -p1 -d "$dir" < "$p" || return 1
    done
  done
  # Eigen as the archive of the commit TensorFlow pins (tools/cmake/modules/
  # eigen.cmake): cloning
  # gitlab.com over git kept failing ("GitLab is currently unable to handle
  # this request"), and a FETCHCONTENT_SOURCE_DIR also skips FetchContent's
  # own patch step.
  local t="$WS/dist/eigen-$TFL_EIGEN_COMMIT.tar.gz" e="$SRCROOT/_qnx_deps/eigen"
  [ -f "$t" ] || curl -fsSL -o "$t" "https://gitlab.com/libeigen/eigen/-/archive/$TFL_EIGEN_COMMIT/eigen-$TFL_EIGEN_COMMIT.tar.gz" || return 1
  echo "$TFL_EIGEN_SHA256  $t" | sha256sum -c --status || { echo "checksum mismatch: $t"; rm -f "$t"; return 1; }
  # deps/fetchcontent/eigen.patch is not applied: it adds the
  # numext::get_integer_by_size helpers, which Eigen has itself at the commit
  # TensorFlow 2.16.1 pins -- applied (with fuzz) it defined them twice.
  mkdir -p "$e" && tar xzf "$t" -C "$e" --strip-components=1
}
TFL_EIGEN_COMMIT=aa6964bf3a34fd607837dd8123bc42465185c4f8
TFL_EIGEN_SHA256=35ba771e30c735a4215ed784d7e032086cf89fe6622dce4d793c45dd74373362

# The QNX patch makes the XNNPACK and pthreadpool downloads run
# ${XNNPACK_PATCH_CMD} / ${PTHREADPOOL_PATCH_CMD} but defines neither, so the
# fetched XNNPACK went unpatched ("Unrecognized CMAKE_SYSTEM_NAME value QNX").
# They are given here, as CMake lists.
# TFLite builds the abseil, Eigen, flatbuffers, ruy, cpuinfo, ... it pins and
# installs them with itself (its CMake package needs them exported); in the
# common prefix they overwrote the abseil, eigen, flatbuffers, ... components
# (abseil 2308 headers and dev links over 20240722). It gets a prefix of its
# own instead, as ROS does, with $ORIGIN as the run path so its libraries find
# the copies beside them: /usr/local/opt/tensorflow-lite (link with
# -L.../opt/tensorflow-lite/lib, or point CMake at its lib/cmake).
TFL_PREFIX="$PREFIX/opt/tensorflow-lite"
do_configure() {
  local d="$SRCROOT/_qnx_deps"
  # The defines qnx-ports' build passes too: farmhash otherwise looks for
  # <endian.h>, which QNX does not have. CFLAGS/CXXFLAGS add to the
  # toolchain's flags (CMAKE_*_FLAGS_INIT) rather than replacing them.
  _tfl_private_names > "$B/qnx-private-names.cmake" || return 1
  CFLAGS="-DFARMHASH_LITTLE_ENDIAN -D__LITTLE_ENDIAN__" CXXFLAGS="-DFARMHASH_LITTLE_ENDIAN -D__LITTLE_ENDIAN__" \
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_PROJECT_tensorflow-lite_INCLUDE="$B/qnx-private-names.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$TFL_PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_INSTALL_RPATH='$ORIGIN' -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
    -DTFLITE_ENABLE_INSTALL=ON -DABSL_ENABLE_INSTALL=ON -DBUILD_SHARED_LIBS=ON \
    -DTFLITE_ENABLE_XNNPACK=ON -DXNNPACK_ENABLE_ASSEMBLY=OFF -DTFLITE_KERNEL_TEST=OFF \
    -DTFLITE_ENABLE_GPU=OFF -DTFLITE_ENABLE_NNAPI=OFF \
    -DQNX_PATCH_DIR="$RECIPE_DIR/tensorflow-lite/deps/fetchcontent" \
    -DXNNPACK_PATCH_CMD="patch;-p1;-i;$RECIPE_DIR/tensorflow-lite/deps/fetchcontent/XNNPACK.patch" \
    -DPTHREADPOOL_PATCH_CMD="patch;-p1;-i;$RECIPE_DIR/tensorflow-lite/deps/fetchcontent/pthreadpool.patch" \
    -DFETCHCONTENT_SOURCE_DIR_ABSEIL-CPP="$d/abseil-cpp" \
    -DFETCHCONTENT_SOURCE_DIR_CPUINFO="$d/cpuinfo" \
    -DFETCHCONTENT_SOURCE_DIR_RUY="$d/ruy" \
    -DFETCHCONTENT_SOURCE_DIR_FARMHASH="$d/farmhash" \
    -DFETCHCONTENT_SOURCE_DIR_EIGEN="$d/eigen" \
    -DFETCHCONTENT_SOURCE_DIR_ML_DTYPES="$d/ml_dtypes" \
    -DPython3_EXECUTABLE=/usr/bin/python3
}
do_build() { cmake --build "$B" -j"$(nproc)"; }
# TFLite's XNNPACK, pthreadpool and farmhash are shared libraries with the
# file names -- and sonames -- of the xnnpack, pthreadpool and farmhash
# components, which are other versions of the same code. QNX's loader puts
# LD_LIBRARY_PATH ahead of a library's own run path (DT_RPATH or DT_RUNPATH),
# so with /usr/local/lib on it TFLite would load those instead. TFLite's are
# renamed libtflite-<name>.so, after its CMakeLists has created them (a
# deferred call, from a project include). Linking them statically instead is
# refused by TFLite's exported CMake package.
_tfl_private_names() {
  cat <<'EOF'
function(_qnx_private_names)
  foreach(t XNNPACK pthreadpool farmhash)
    if(TARGET ${t})
      set_target_properties(${t} PROPERTIES OUTPUT_NAME "tflite-${t}")
    endif()
  endforeach()
endfunction()
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL _qnx_private_names)
EOF
}
# TFLite's install rules also leave its XNNPACK library out; in the common
# prefix the xnnpack component's libXNNPACK.so stood in for it ("undefined
# reference to xnn_experiment_enable_adaptive_avx_optimization"). Install its
# own, which has the install run path already (CMAKE_BUILD_WITH_INSTALL_RPATH).
do_install() {
  DESTDIR="$STAGE" cmake --install "$B"
  find "$B/_deps" -name 'libtflite-XNNPACK.so*' -exec cp -a {} "$STAGE$TFL_PREFIX/lib/" \;
  [ -e "$STAGE$TFL_PREFIX/lib/libtflite-XNNPACK.so" ]
}
