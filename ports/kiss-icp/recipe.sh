# kiss-icp: KISS-ICP LiDAR odometry -- point-to-point ICP with an adaptive
# correspondence threshold, motion compensation (deskewing) and a voxel hash
# map as local map. This is the C++ core (cpp/kiss_icp of the repository):
# libkiss_icp_core (voxel map, registration, preprocessing, threshold),
# libkiss_icp_pipeline (kiss_icp::pipeline::KissICP, the odometry itself)
# and libkiss_icp_metrics (KITTI sequence errors), on Eigen, Sophus, oneTBB
# and tsl::robin_map. The Python bindings and the ROS 2 wrapper of the
# repository are not part of this port.
#
# - The libraries are static and position independent, as upstream builds
#   them (its wrappers link them into their own modules).
# - Upstream has no install step (it is meant to be add_subdirectory()'d);
#   patch 0001 adds one: the three archives, the headers under
#   include/kiss_icp/{core,pipeline,metrics}/ -- the layout the headers
#   themselves include each other by -- and a CMake package kiss_icp
#   (kiss_icp::core, kiss_icp::pipeline, kiss_icp::metrics) that finds the
#   four dependencies again.
# - The four dependencies come from this workspace; upstream would otherwise
#   download and build its own copies (FetchContent) when one is not found.
#   FETCHCONTENT_FULLY_DISCONNECTED makes that an error instead of a
#   download during the build.
# - USE_CCACHE=OFF: no compiler launcher from the build machine.
# - The repository's top-level Makefile is for the Python package ("make"
#   runs pip install); the build starts in cpp/kiss_icp instead.
VERSION=1.3.0; GIT=https://github.com/PRBonn/kiss-icp.git; REF=v1.3.0
DEPS="eigen sophus onetbb tsl-robin-map"
BUILDSYS=cmake
SUBDIR=cpp/kiss_icp
CMAKE_ARGS_EXTRA=( -DKISS_ICP_INSTALL=ON -DUSE_CCACHE=OFF
                   -DUSE_SYSTEM_EIGEN3=ON -DUSE_SYSTEM_SOPHUS=ON -DUSE_SYSTEM_TBB=ON
                   -DUSE_SYSTEM_TSL-ROBIN-MAP=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON )
