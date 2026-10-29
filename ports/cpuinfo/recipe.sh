# pytorch/cpuinfo has no releases; pinned to the commit the qnx-ports QNX branch is based on.
VERSION=0-git-20240417; GIT=https://github.com/pytorch/cpuinfo.git; COMMIT=3c8b1533ac03dd6531ab6e7b9245d488f13a82a5
CMAKE_ARGS_EXTRA=( -DCPUINFO_BUILD_TOOLS=OFF -DCPUINFO_BUILD_UNIT_TESTS=OFF -DCPUINFO_BUILD_MOCK_TESTS=OFF
                   -DCPUINFO_BUILD_BENCHMARKS=OFF -DCPUINFO_LIBRARY_TYPE=shared -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
