VERSION=3.7.7; GIT=https://github.com/libarchive/libarchive.git; REF=v3.7.7
# Also linked at run time (found by scripts/gen-dependency-index): expat lz4
DEPS="zlib bzip2 xz zstd openssl libxml2 expat lz4"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DENABLE_TEST=OFF -DENABLE_CPIO=OFF
                   -DENABLE_CAT=OFF -DENABLE_TAR=ON -DENABLE_ACL=OFF
                   -DENABLE_ICONV=OFF -DENABLE_PCREPOSIX=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
