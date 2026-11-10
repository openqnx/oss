# Gloo: collective communications for machine learning (allreduce,
# broadcast, barrier, allgather, ... across processes), PyTorch's CPU
# distributed backend. Built with its TCP transport (epoll; QNX has it in
# libepoll -- QNX changes in patches/, after qnx-ports' qnx-mediapipe branch)
# and the file/prefix/hash rendezvous stores; no CUDA, ROCm, ibverbs, Redis
# or MPI. Upstream has no releases: pinned to a commit of main.
VERSION=0_git20261104; GIT=https://github.com/facebookincubator/gloo.git; COMMIT=8d8f8645ed5ef28aeca751bf17119efab5de2244
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_TEST=OFF -DBUILD_BENCHMARK=OFF -DUSE_REDIS=OFF -DUSE_IBVERBS=OFF -DUSE_MPI=OFF -DUSE_LIBUV=OFF )
