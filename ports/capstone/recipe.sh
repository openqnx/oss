# capstone: multi-architecture disassembly framework (x86, Arm, AArch64, RISC-V, ...).
VERSION=5.0.9; GIT=https://github.com/capstone-engine/capstone.git; REF=5.0.9
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DCAPSTONE_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=ON -DCAPSTONE_BUILD_CSTOOL=ON )
