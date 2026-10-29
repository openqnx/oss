VERSION=24.3.25; GIT=https://github.com/google/flatbuffers.git; REF=v24.3.25
# flatc is built for the target too: schemas can then be compiled on the board.
# Generate code on the host with a native flatc of the same version.
CMAKE_ARGS_EXTRA=( -DFLATBUFFERS_BUILD_TESTS=OFF -DFLATBUFFERS_BUILD_FLATHASH=OFF
                   -DFLATBUFFERS_BUILD_SHAREDLIB=ON -DFLATBUFFERS_BUILD_FLATC=ON )
