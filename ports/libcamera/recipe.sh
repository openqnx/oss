VERSION=0.5.1+rpt20250722; GIT=https://github.com/raspberrypi/libcamera.git; REF=v0.5.1+rpt20250722
# libcamera's Raspberry Pi image processing algorithms (auto white balance,
# auto exposure) for the Camera Module 3 on QNX -- the part qnx-ports ports,
# built as librpi_camera_ipa with qnx-ports' own CMakeLists (files/), not
# libcamera's Meson build (whose device layer is V4L2/Linux). patches/: the
# QNX changes from qnx-ports/libcamera. Raspberry Pi 4/5 only: aarch64le.
ARCHES_SUPPORTED="aarch64le"
DEPS="libyaml"
BUILDSYS=custom
do_configure() {
  cmake -S "$RECIPE_DIR/libcamera/files" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DPROJECT_ROOT="$S"
}
do_build() { cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
