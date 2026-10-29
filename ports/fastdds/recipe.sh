VERSION=2.14.4; GIT=https://github.com/eProsima/Fast-DDS.git; REF=v2.14.4
DEPS="fastcdr foonathan_memory tinyxml2 openssl"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DTHIRDPARTY=ON -DCOMPILE_EXAMPLES=OFF
                   -DBUILD_DOCUMENTATION=OFF -DSECURITY=ON
                   -DTHIRDPARTY_Asio=FORCE -DTHIRDPARTY_TinyXML2=OFF
                   -DOPENSSL_ROOT_DIR="$STAGE$PREFIX"
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
