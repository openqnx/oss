VERSION=3.5.6; GIT=https://github.com/COVESA/vsomeip.git; REF=3.5.6
DEPS="boost"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DENABLE_SIGNAL_HANDLING=1
                   -DBoost_NO_BOOST_CMAKE=OFF
                   -DBOOST_ROOT="$STAGE$PREFIX"
                   -DBoost_INCLUDE_DIR="$STAGE$PREFIX/include"
                   -DBoost_LIBRARY_DIR="$STAGE$PREFIX/lib"
                   -DDISABLE_DLT=1 )
