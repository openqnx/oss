# Pinned to the GKlib commit qnx-ports builds METIS against.
# GKlib: the support library METIS is built on (same authors). No releases.
VERSION=5.1.1-git-20230326; GIT=https://github.com/KarypisLab/GKlib.git; COMMIT=8bd6bad750b2b0d90800c632cf18e8ee93ad72d7
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DOPENMP=OFF )
