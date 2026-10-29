VERSION=2.3.0; GIT=https://github.com/csmith-project/csmith.git; REF=csmith-2.3.0
# csmith 2.3.0 still uses std::bind2nd/std::ptr_fun, which C++17 removed. The
# QNX toolchain builds as gnu++17 by default; libc++ keeps the old binders
# available when asked to. (CMAKE_CXX_FLAGS replaces the toolchain's initial
# flags, so those are repeated here.)
CMAKE_ARGS_EXTRA=( "-DCMAKE_CXX_FLAGS=-std=gnu++17 -isystem $STAGE$PREFIX/include -D_LIBCPP_ENABLE_CXX17_REMOVED_BINDERS" )
