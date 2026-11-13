# opencl-headers: the Khronos OpenCL C API headers (CL/cl.h, CL/cl_ext.h,
# CL/cl_icd.h, CL/cl_layer.h ... for OpenCL 1.0 to 3.0), with their CMake
# package (OpenCL::Headers) and OpenCL-Headers.pc. Header only: what
# opencl-icd-loader (libOpenCL.so), pocl and every OpenCL program compile
# against. An application selects the API version it is written for with
# -DCL_TARGET_OPENCL_VERSION=<100..300>; without it the headers default to
# 3.0 and say so in a #pragma message.
#
# Nothing had to change for QNX: cl_platform.h takes its fixed-width types
# from <stdint.h> on every non-Windows compiler. The upstream tests are host
# programs and are left out.
VERSION=2026.05.29; GIT=https://github.com/KhronosGroup/OpenCL-Headers.git; REF=v2026.05.29
DEPS=""
# computelibrary bundles an older copy of these headers; its recipe leaves
# them out and depends on this component.
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_TESTING=OFF -DOPENCL_HEADERS_BUILD_TESTING=OFF -DOPENCL_HEADERS_BUILD_CXX_TESTS=OFF )
