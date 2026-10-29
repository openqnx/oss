# Header-only. v1.1.0 (2016) is the last tagged release; upstream development
# continues on master and every distribution ships a snapshot of it.
VERSION=1.1.0-24b5e7a; GIT=https://github.com/Tencent/rapidjson.git; REF=
do_fetch() { git clone https://github.com/Tencent/rapidjson.git "$SRCROOT" && git -C "$SRCROOT" checkout 24b5e7a8b27f42fa16b96fc70aade9106cf7102f; }
CMAKE_ARGS_EXTRA=( -DRAPIDJSON_BUILD_DOC=OFF -DRAPIDJSON_BUILD_EXAMPLES=OFF -DRAPIDJSON_BUILD_TESTS=OFF )
