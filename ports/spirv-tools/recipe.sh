# spirv-tools: the SPIR-V assembler, disassembler, validator, optimizer and
# linker (spirv-as, spirv-dis, spirv-val, spirv-opt, spirv-link ...) and their
# library. Pinned to the same Vulkan SDK release as spirv-headers and glslang.
VERSION=1.4.321.0; GIT=https://github.com/KhronosGroup/SPIRV-Tools.git; REF=vulkan-sdk-1.4.321.0
DEPS="spirv-headers"
BUILDSYS=cmake
# - the staged spirv-headers instead of a copy in external/ (the generated
#   tables come from its grammar files);
# - the generators are Python scripts run on the build machine;
# - shared libraries only (as distributions ship it: glslang and shaderc link
#   libSPIRV-Tools-opt.so), no tests, no -Werror (GCC 12 warnings).
CMAKE_ARGS_EXTRA=( -DSPIRV-Headers_SOURCE_DIR="$STAGE$PREFIX"
                   -DPython3_EXECUTABLE=/usr/bin/python3
                   -DBUILD_SHARED_LIBS=ON -DSPIRV_TOOLS_BUILD_STATIC=OFF
                   -DSPIRV_SKIP_TESTS=ON -DSPIRV_WERROR=OFF -DBUILD_TESTING=OFF )
