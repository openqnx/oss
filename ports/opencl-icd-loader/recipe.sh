# opencl-icd-loader: the Khronos OpenCL ICD loader, libOpenCL.so.1 -- the
# library OpenCL programs link. It implements no OpenCL itself: at the first
# call it reads the *.icd files of the installed implementations (each names
# a driver library, e.g. pocl's), loads them and dispatches every call to
# the platform it belongs to. With no implementation installed
# clGetPlatformIDs() returns CL_PLATFORM_NOT_FOUND_KHR (-1001). Also
# cllayerinfo, which lists the installed OpenCL layers.
#
# Upstream supports QNX (it builds its "linux" back end there: dlopen(),
# readdir(), and getenv() ignored in set-id programs). What is changed:
#  - ICD files are looked up in /usr/local/etc/OpenCL/vendors first, then in
#    /etc/OpenCL/vendors (layers: /etc/OpenCL/layers, then
#    /usr/local/etc/OpenCL/layers): the components here install under
#    /usr/local and never write to the base system's /etc
#    (patches/0001, OPENCL_ICD_LOADER_EXTRA_CONFIG_DIR). OCL_ICD_VENDORS=<dir>
#    replaces both directories, OCL_ICD_FILENAMES=<lib>[:<lib>] adds drivers
#    by name, OCL_ICD_ENABLE_TRACE=1 prints what the loader does.
#  - the headers are opencl-headers' (same release), not a copy in inc/.
#  - upstream's test programs are built and installed in
#    libexec/opencl-icd-loader/: a stub driver (libOpenCLDriverStub.so, every
#    entry point logs its arguments), its ICD 2 variant, a layer
#    (libPrintLayer.so) and icd_loader_test, which calls the whole API through
#    the loader and compares what the application passed with what the driver
#    received. They are how the loader is tested on a target that has no
#    OpenCL implementation (tests/src/test_opencl_icd_loader.c). They carry
#    no run path (CMake would write the build directory into them): run them
#    with that directory in LD_LIBRARY_PATH, for libIcdLog.so.
VERSION=2026.05.29; GIT=https://github.com/KhronosGroup/OpenCL-ICD-Loader.git; REF=v2026.05.29
DEPS="opencl-headers"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DOPENCL_ICD_LOADER_EXTRA_CONFIG_DIR="$PREFIX/etc/OpenCL"
                   -DBUILD_TESTING=ON -DOPENCL_ICD_LOADER_BUILD_TESTING=ON
                   -DCMAKE_SKIP_RPATH=ON )
do_install() {
  DESTDIR="$STAGE" cmake --install "$B"
  local d="$STAGE$PREFIX/libexec/opencl-icd-loader"
  install -d "$d"
  install -m755 "$B/icd_loader_test" "$B/libIcdLog.so" "$B/libOpenCLDriverStub.so" "$B/libOpenCLDriverStubICD2.so" \
                "$B/test/layer/libPrintLayer.so" "$d/"
}
