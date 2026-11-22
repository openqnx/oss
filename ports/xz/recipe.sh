VERSION=5.8.4; GIT=https://github.com/tukaani-project/xz.git; REF=v5.8.4
DEPS="gettext"   # links -lintl; undeclared, it found the SDP's libintl
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF
                   -DXZ_NLS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
