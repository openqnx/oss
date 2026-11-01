# llama.cpp: LLM inference in C/C++ on ggml -- the llama library, llama-cli,
# llama-server, llama-quantize and the other tools, CPU inference -- with
# QNX's change (patches/0001, from github.com/qnx-ports/llama.cpp, branch
# qnx-8.0; release b9006 is the commit QNX's port is based on).
VERSION=0_b9006; GIT=https://github.com/ggml-org/llama.cpp.git; REF=b9006
DEPS=""
BUILDSYS=cmake
# It carries its own ggml (libggml*.so, ggml.h), as stable-diffusion.cpp and
# whisper.cpp each do, in versions that are not interchangeable: each gets a
# prefix of its own, /usr/local/opt/llama.cpp, its commands linked into bin/.
# The run path is that prefix's lib/, absolute: QNX's loader does not expand
# $ORIGIN (docs/porting-hazards.md).
# No libcurl (models are given as files, not downloaded), no OpenMP (QNX's GCC
# has no OpenMP runtime: ggml's own thread pool), no Vulkan (not in the SDP).
OPT="$PREFIX/opt/llama.cpp"
CMAKE_ARGS_EXTRA=( -DCMAKE_INSTALL_PREFIX="$OPT" -DCMAKE_INSTALL_RPATH="$OPT/lib" -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON -DBUILD_SHARED_LIBS=ON
                   -DLLAMA_BUILD_TESTS=OFF -DLLAMA_BUILD_EXAMPLES=OFF -DLLAMA_BUILD_TOOLS=ON -DLLAMA_BUILD_SERVER=ON -DLLAMA_CURL=OFF
                   -DGGML_NATIVE=OFF -DGGML_OPENMP=OFF -DGGML_VULKAN=OFF -DGGML_BACKEND_DL=OFF -DLLAMA_BUILD_NUMBER=9006 )
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  local p; mkdir -p "$STAGE$PREFIX/bin"
  for p in "$STAGE$OPT"/bin/llama-*; do [ -e "$p" ] && ln -sf "../opt/llama.cpp/bin/${p##*/}" "$STAGE$PREFIX/bin/${p##*/}"; done; return 0
}
