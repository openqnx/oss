# The release aws-crt-cpp v0.38.5 pins as its submodule.
VERSION=1.7.0; GIT=https://github.com/aws/s2n-tls.git; REF=v1.7.0
DEPS="openssl"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DUNSAFE_TREAT_WARNINGS_AS_ERRORS=OFF -DBUILD_TESTING=OFF )
