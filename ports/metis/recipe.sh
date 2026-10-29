VERSION=5.2.1; GIT=https://github.com/KarypisLab/METIS.git; REF=v5.2.1
DEPS="gklib"
CMAKE_ARGS_EXTRA=( -DCMAKE_INSTALL_INCLUDEDIR=include -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_BINDIR=bin -DGKLIB_PATH="$STAGE$PREFIX" -DSHARED=ON )
# METIS's CMake build expects "make config" to have run first: it writes
# build/xinclude/metis.h with the index and real widths (32-bit here, as
# upstream's default) and copies the header's CMakeLists next to it.
do_patch() {
  cd "$SRCROOT" && mkdir -p build/xinclude && \
  { echo "#define IDXTYPEWIDTH 32"; echo "#define REALTYPEWIDTH 32"; cat include/metis.h; } > build/xinclude/metis.h && \
  cp include/CMakeLists.txt build/xinclude/
}
