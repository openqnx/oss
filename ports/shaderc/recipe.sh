# shaderc: Google's shader compiler collection -- glslc (a GCC/Clang-like
# command line GLSL/HLSL -> SPIR-V compiler) and libshaderc, the API Vulkan
# applications and ffmpeg/libplacebo/mpv compile shaders with.
VERSION=2026.2; GIT=https://github.com/google/shaderc.git; REF=v2026.2
DEPS="glslang spirv-tools spirv-headers"
BUILDSYS=cmake
# Built against the staged glslang and SPIRV-Tools rather than source trees in
# third_party/ (patch 0001: SHADERC_USE_INSTALLED_DEPS, as distributions do).
# No tests (gmock), examples or copyright check; Python on the build machine.
CMAKE_ARGS_EXTRA=( -DSHADERC_USE_INSTALLED_DEPS=ON -DSHADERC_SKIP_TESTS=ON -DSHADERC_SKIP_EXAMPLES=ON
                   -DSHADERC_SKIP_COPYRIGHT_CHECK=ON -DSHADERC_ENABLE_WGSL_OUTPUT=OFF
                   -DBUILD_SHARED_LIBS=ON -DPython_EXECUTABLE=/usr/bin/python3 )
