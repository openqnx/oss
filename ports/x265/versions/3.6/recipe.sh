VERSION=3.6; GIT=https://bitbucket.org/multicoreware/x265_git.git; REF=3.6
SUBDIR=source
CMAKE_ARGS_EXTRA=( -DENABLE_SHARED=ON -DENABLE_CLI=OFF -DENABLE_TESTS=OFF
                   -DENABLE_ASSEMBLY=OFF          # nasm/yasm absent on this host
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
