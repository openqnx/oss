# miniz: single-file zlib/deflate replacement with ZIP archive reading and writing.
VERSION=3.1.2; GIT=https://github.com/richgel999/miniz.git; REF=3.1.2
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF -DBUILD_FUZZERS=OFF )
