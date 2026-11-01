# Google's low-precision (8-bit) matrix multiplication library. No releases;
# pinned to the last commit. Used header-only (as TensorFlow does): the
# headers go to include/gemmlowp/. Upstream's contrib/CMakeLists.txt builds
# only its tests and benchmarks and installs nothing.
VERSION=0-git-20231103; GIT=https://github.com/google/gemmlowp.git; COMMIT=16e8662c34917be0065110bfcd9cc27d30f52fdf
BUILDSYS=custom
do_build() { :; }
do_install() {
  local d
  for d in fixedpoint internal meta profiling public eight_bit_int_gemm; do
    install -d "$STAGE$PREFIX/include/gemmlowp/$d" || return 1
    find "$S/$d" -maxdepth 1 -name '*.h' -exec install -m644 {} "$STAGE$PREFIX/include/gemmlowp/$d/" \; || return 1
  done
}
