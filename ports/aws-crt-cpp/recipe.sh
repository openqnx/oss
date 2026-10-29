VERSION=0.38.5; GIT=https://github.com/awslabs/aws-crt-cpp.git; REF=v0.38.5
DEPS="aws-c-auth aws-c-mqtt aws-c-event-stream aws-c-s3"
# BUILD_DEPS=OFF: build against the separately packaged aws-c-* components
# instead of the copies in crt/ (whose submodules are therefore not needed).
do_fetch() { git -c advice.detachedHead=false clone -q --depth 1 -b "$REF" "$GIT" "$SRCROOT"; }
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_DEPS=OFF -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" -DAWS_WARNINGS_ARE_ERRORS=OFF )
