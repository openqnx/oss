# The release aws-crt-cpp v0.38.5 pins as its submodule, so the
# whole AWS C runtime is one consistent set.
VERSION=0.26.1; GIT=https://github.com/awslabs/aws-c-io.git; REF=v0.26.1
DEPS="aws-c-common aws-c-cal s2n-tls"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" -DAWS_WARNINGS_ARE_ERRORS=OFF  )
