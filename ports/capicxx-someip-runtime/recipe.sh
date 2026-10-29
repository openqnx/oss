VERSION=3.2.4; GIT=https://github.com/COVESA/capicxx-someip-runtime.git; REF=3.2.4
DEPS="capicxx-core-runtime vsomeip"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                   -DUSE_INSTALLED_COMMONAPI=ON
                   -DCommonAPI_DIR="$STAGE$PREFIX/lib/cmake/CommonAPI"
                   -Dvsomeip3_DIR="$STAGE$PREFIX/lib/cmake/vsomeip3" )
