VERSION=20240722.0; GIT=https://github.com/abseil/abseil-cpp.git; REF=20240722.0
BUILDSYS=cmake
# One abseil for the whole tree.  protobuf and gRPC both vendor abseil and both
# used to install their copy -- 20240116 (sonames .2401) from protobuf, 20240722
# (.2407) from gRPC -- so 86 libabsl_* libraries were staged twice, the
# unversioned symlinks pointed at whichever installed last, and a gRPC process
# mapped both.  scripts/check-dupes now catches that.  This is the version gRPC
# 1.69 pins, and the same one ONNX Runtime vendors (statically, so it does not
# take part in this).
#
# The patch is the libc++ fix gRPC and ORT already needed: MaskedPointer has no
# default constructor and libc++'s std::atomic<T> requires one.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DABSL_BUILD_TESTING=OFF
                   -DABSL_PROPAGATE_CXX_STD=ON -DCMAKE_CXX_STANDARD=17
                   -DABSL_ENABLE_INSTALL=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
