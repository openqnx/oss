VERSION=1.18.0; GIT=https://github.com/google/googletest.git; REF=v1.18.0
# googletest needs no other component, so no staged header either. The
# toolchain file puts the staging tree's include directory on every compile
# with -isystem, ahead of googletest's own include directories, which its
# CMakeLists also marks SYSTEM (so a -I for them is merged into the same
# list and does not help). On an update the installed headers of the version
# before then win: 1.18's gmock-actions.h met 1.17's gtest-port.h, which has
# no GTEST_INTERNAL_DEPRECATE_AND_INLINE ("expected constructor, destructor,
# or type conversion before '('"). Flags without the staging tree instead.
CMAKE_ARGS_EXTRA=( -Dgtest_force_shared_crt=ON -DBUILD_SHARED_LIBS=ON
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                   -DCMAKE_C_FLAGS= -DCMAKE_CXX_FLAGS=-std=gnu++17 )
