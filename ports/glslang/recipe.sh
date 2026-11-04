# glslang: the Khronos reference GLSL/ESSL (and HLSL) front end and SPIR-V
# generator -- glslangValidator (glslang) and spirv-remap, and the libraries
# shaderc and Vulkan applications compile shaders with.
VERSION=1.4.321.0; GIT=https://github.com/KhronosGroup/glslang.git; REF=vulkan-sdk-1.4.321.0
DEPS="spirv-tools spirv-headers"
BUILDSYS=cmake
# - the staged SPIRV-Tools (the same Vulkan SDK release) for -O/-Os, instead
#   of a copy checked out into External/;
# - shared libraries, HLSL input on, no tests; Python runs the build-info
#   generator on the build machine.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DALLOW_EXTERNAL_SPIRV_TOOLS=ON -DBUILD_EXTERNAL=OFF
                   -DENABLE_OPT=ON -DENABLE_HLSL=ON -DENABLE_GLSLANG_BINARIES=ON
                   -DGLSLANG_TESTS=OFF -DBUILD_TESTING=OFF -DENABLE_PCH=OFF
                   -DPython3_EXECUTABLE=/usr/bin/python3 )
