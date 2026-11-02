VERSION=20260817.0; GIT=https://github.com/abseil/abseil-cpp.git; REF=20260817.0
BUILDSYS=cmake
# One abseil for the whole tree.  protobuf and gRPC both vendor abseil and both
# used to install their copy -- 20240116 (sonames .2401) from protobuf, 20240722
# (.2407) from gRPC -- so 86 libabsl_* libraries were staged twice, the
# unversioned symlinks pointed at whichever installed last, and a gRPC process
# mapped both.  scripts/check-dupes now catches that.  20240722 was the version
# gRPC 1.69 pins (kept in versions/20240722.0/); ONNX Runtime vendors its own
# copy statically, so it does not take part in this.
#
# No QNX patches since 20260817.0: the one 20240722.0 needed (a default
# constructor for absl's MaskedPointer, which libc++'s std::atomic<T> requires)
# is upstream now.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DABSL_BUILD_TESTING=OFF
                   -DABSL_PROPAGATE_CXX_STD=ON -DCMAKE_CXX_STANDARD=17
                   -DABSL_ENABLE_INSTALL=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
