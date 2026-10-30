# mimalloc: Microsoft's compact general-purpose allocator (link it, or preload
# it to replace malloc), with QNX's changes (patches/0001, from
# github.com/qnx-ports/mimalloc).
VERSION=3.1.5; GIT=https://github.com/microsoft/mimalloc.git; REF=v3.1.5
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DMI_BUILD_TESTS=OFF -DMI_BUILD_OBJECT=OFF -DMI_OVERRIDE=OFF )
