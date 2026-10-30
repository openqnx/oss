# stable-diffusion.cpp: Stable Diffusion / FLUX image generation in C/C++ on
# ggml, CPU inference (the sd command and the library), with QNX's changes
# (patches/0001, from github.com/qnx-ports/stable-diffusion.cpp; the commit
# QNX's port is based on).
VERSION=0_git20260601; GIT=https://github.com/leejet/stable-diffusion.cpp.git; COMMIT=7948df8ac1070f5f6881b8d34675821893eb97d6
DEPS=""
BUILDSYS=cmake
# It carries its own ggml (libggml*.so, ggml.h), as stable-diffusion.cpp and
# whisper.cpp each do, in versions that are not interchangeable: each gets a
# prefix of its own, /usr/local/opt/stable-diffusion.cpp, its commands linked into bin/.
# The run path is that prefix's lib/, absolute: QNX's loader does not expand
# $ORIGIN (docs/porting-hazards.md).
OPT="$PREFIX/opt/stable-diffusion.cpp"
CMAKE_ARGS_EXTRA=( -DCMAKE_INSTALL_PREFIX="$OPT" -DCMAKE_INSTALL_RPATH="$OPT/lib" -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON -DSD_BUILD_SHARED_LIBS=ON -DSD_BUILD_EXAMPLES=ON -DGGML_NATIVE=OFF -DGGML_OPENMP=OFF -DGGML_VULKAN=OFF )
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  local p; mkdir -p "$STAGE$PREFIX/bin"
  for p in sd-cli sd-server; do [ -e "$STAGE$OPT/bin/$p" ] && ln -sf "../opt/stable-diffusion.cpp/bin/$p" "$STAGE$PREFIX/bin/$p"; done; return 0
}
