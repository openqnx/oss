VERSION=1.4.8; GIT=https://github.com/open62541/open62541.git; REF=v1.4.8
DEPS="openssl"
# The node-set and data-type code generators are Python scripts that run on the
# build host. The QNX toolchain file restricts find_package to the target
# sysroot, so name the host interpreter explicitly.
# The library is assembled from CMake OBJECT libraries, which do not inherit
# PIC from BUILD_SHARED_LIBS; on x86_64 the shared link then fails with
# "relocation R_X86_64_PC32 ... recompile with -fPIC".
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DUA_ENABLE_ENCRYPTION=OPENSSL -DUA_BUILD_EXAMPLES=OFF
                   -DUA_BUILD_UNIT_TESTS=OFF -DUA_FORCE_WERROR=OFF -DUA_ENABLE_AMALGAMATION=OFF
                   -DPython3_EXECUTABLE=/usr/bin/python3
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
