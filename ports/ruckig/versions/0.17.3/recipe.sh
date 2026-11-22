# Ruckig: instantaneous, time-optimal and jerk-limited trajectory generation
# for robots and machines (the Community Version: online trajectories for any
# number of degrees of freedom, with velocity, acceleration and jerk limits).
# MoveIt 2 smooths its trajectories with it. The library only: its tests,
# examples and benchmark (doctest, nlohmann/json bundled for them) and the
# Python module are not built; nor is the cloud client (BUILD_CLOUD_CLIENT,
# which computes Ruckig Pro trajectories on a remote server over HTTP): it
# is compiled with Ruckig's bundled nlohmann/json and exports functions with
# that JSON's types, while consumers' <nlohmann/json.hpp> is the
# nlohmann_json component's (another ABI tag): MoveIt's
# libmoveit_trajectory_processing then failed to load ("unknown symbol:
# ruckig::CloudClient::post(nlohmann::json_abi_v3_12_0::...)").
VERSION=0.17.3; GIT=https://github.com/pantor/ruckig.git; REF=v0.17.3
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_BENCHMARK=OFF -DBUILD_PYTHON_MODULE=OFF -DBUILD_CLOUD_CLIENT=OFF -DBUILD_SHARED_LIBS=ON )
