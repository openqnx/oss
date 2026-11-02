# btop: resource monitor (CPU, memory, disks, network, processes), with QNX's
# QNX collector (patches/0001, from github.com/qnx-ports/btop).
VERSION=1.4.6; GIT=https://github.com/aristocratos/btop.git; REF=v1.4.6
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBTOP_GPU=OFF -DBTOP_LTO=OFF )
