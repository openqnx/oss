# spirv-headers: the SPIR-V registry's headers and grammar files (spirv.h,
# spirv.hpp, the JSON grammars) that SPIRV-Tools, glslang and shaderc are built
# from. Header and data only. The tag is the Vulkan SDK release the rest of the
# shader toolchain (spirv-tools, glslang) is pinned to.
VERSION=1.4.321.0; GIT=https://github.com/KhronosGroup/SPIRV-Headers.git; REF=vulkan-sdk-1.4.321.0
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_TESTING=OFF -DSPIRV_HEADERS_ENABLE_TESTS=OFF )
