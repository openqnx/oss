VERSION=0.3.28; GIT=https://github.com/OpenMathLib/OpenBLAS.git; REF=v0.3.28
# Built with OpenBLAS's CMake build, as qnx-ports does: the Makefile build
# queries the compiler with -dumpversion, which qcc does not answer (and then
# fails with "expr: syntax error"). No Fortran: LAPACK comes from its C
# translation (C_LAPACK). One kernel set per architecture -- ARMV8 (NEON) and
# HASWELL (AVX2) -- rather than runtime dispatch.
# The default thread count is capped at the CPU count by patch 0003 -- on
# QNX it was 64, each with a 128 MB buffer QNX commits at once. The buffer
# size stays (a smaller one makes OpenBLAS warn that large GEMMs may crash).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DNOFORTRAN=ON -DC_LAPACK=ON -DBUILD_WITHOUT_LAPACK=OFF
                   -DDYNAMIC_ARCH=OFF -DUSE_OPENMP=0 -DUSE_THREAD=1 -DNUM_THREADS=64 -DBUILD_TESTING=OFF
                   "-DTARGET=$([ "$QNX_ARCH" = aarch64le ] && echo ARMV8 || echo HASWELL)" )
