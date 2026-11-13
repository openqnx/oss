# hdf5: the HDF5 library and tools -- a self-describing binary file format
# for large numerical data (groups, n-dimensional datasets, attributes,
# compound types, chunking and compression filters). Built here: the C
# library (libhdf5), the high-level library (libhdf5_hl: H5LT, H5TB, H5IM,
# H5DS, H5PT), the C++ API (libhdf5_cpp, libhdf5_hl_cpp) and the command
# line tools (h5dump, h5ls, h5diff, h5repack, h5copy, h5stat, ...).
#
# QNX specifics:
# - Cross build. HDF5's CMake files run a handful of small programs at
#   configure time to learn how the target converts long double and
#   _Float16 values (config/cmake/ConversionTests.c); they cannot run here.
#   Their answers are given below as cache variables instead of being left
#   to the "cross-compile default" branch, so the configuration is stated in
#   one place and identical on every build machine. For x86_64 and aarch64
#   with GCC 12 they are the answers every Linux build of these two ABIs
#   gets: no IBM POWER special cases, accurate long double <-> long long
#   conversion.
# - _Float16 (H5T_NATIVE_FLOAT16): HDF5 switches it off when it cannot run
#   its probe. The probe's first half is a link test (does libgcc have
#   __extendhfsf2, __truncdfhf2, ...): compiled and linked by hand for both
#   QNX targets, with and without optimisation, it links. The second half
#   guards against a macOS 13 compiler defect that does not apply to GCC.
#   So it is enabled; tests/src/test_hdf5.cpp checks a _Float16 conversion on
#   the target against the IEEE 754 binary16 bit pattern.
# - H5_HAVE_LIBDL=0: dlopen() is in QNX's libc and there is no libdl. The
#   workspace's toolchain setup provides an empty stub libdl.a for projects
#   that insist on -ldl; HDF5's probe finds it and would record "dl" as a
#   library HDF5 needs (link line, libhdf5.settings, hdf5-config.cmake).
# - libhdf5.settings (installed, and compiled into libhdf5 for
#   H5check_version()'s diagnostics) records the build: compiler and
#   archiver paths, link flags, the build directory and the build machine's
#   kernel as "Host system". do_build rewrites those to what is true on the
#   target before anything is compiled, so neither the file nor the library
#   depends on where and when it was built.
# - Shared libraries only (the tools link them); examples and the test
#   suite are not built. Thread-safe mode stays off, as in upstream's
#   default: it cannot be combined with the C++ and high-level APIs.
# - SZIP filter through libaec (encoder and decoder), DEFLATE through zlib.
# - Install layout: CMake package files in lib/cmake/hdf5 and COPYING,
#   RELEASE.txt in share/hdf5 (upstream's defaults are <prefix>/cmake and
#   <prefix>/share itself, shared with every other component).
VERSION=1.14.6; GIT=https://github.com/HDFGroup/hdf5.git; REF=hdf5_1.14.6
DEPS="zlib libaec"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=(
  -DBUILD_TESTING=OFF -DHDF5_BUILD_EXAMPLES=OFF
  -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF
  -DHDF5_BUILD_TOOLS=ON -DHDF5_BUILD_HL_LIB=ON -DHDF5_BUILD_CPP_LIB=ON
  -DHDF5_BUILD_FORTRAN=OFF -DHDF5_BUILD_JAVA=OFF
  -DHDF5_ENABLE_Z_LIB_SUPPORT=ON -DHDF5_ENABLE_SZIP_SUPPORT=ON -DHDF5_ENABLE_SZIP_ENCODING=ON
  -DHDF5_INSTALL_CMAKE_DIR=lib/cmake/hdf5 -DHDF5_INSTALL_DATA_DIR=share/hdf5
  -DH5_DEFAULT_PLUGINDIR=/usr/local/lib/hdf5/plugin
  # dlopen() is in libc (see above)
  -DH5_HAVE_LIBDL=0
  # config/cmake/ConversionTests.c, answered for x86_64 and aarch64 (see above).
  # An empty value is HDF5's own "no" (the macro tests the variable's value).
  -DH5_LDOUBLE_TO_LONG_SPECIAL= -DH5_LONG_TO_LDOUBLE_SPECIAL=
  -DH5_LDOUBLE_TO_LLONG_ACCURATE=1 -DH5_LLONG_TO_LDOUBLE_CORRECT=1
  -DH5_DISABLE_SOME_LDOUBLE_CONV=
  -DH5_FLOAT16_CONVERSION_FUNCS_LINK=1 -DH5_FLOAT16_CONVERSION_FUNCS_LINK_NO_FLAGS=1
  -DH5_LDOUBLE_TO_FLOAT16_CORRECT=1 -DH5_LDOUBLE_TO_FLOAT16_CORRECT_NO_FLAGS=1
)
do_build() {
  # The build record: target facts instead of build-machine paths (see above).
  # Both files are written by configure; H5build_settings.c is libhdf5.settings
  # as a C string.
  local f
  for f in "$B/src/libhdf5.settings" "$B/src/H5build_settings.c"; do
    [ -f "$f" ] || { echo "missing $f"; return 1; }
    # [^\\]*: up to the end of the line in the text file, up to the closing
    # \n" in the C string.
    sed -i -e "s#$QNX_TARGET#<qnx-target>#g" -e "s#$STAGE##g" -e "s#$QNX_HOST/usr/bin/##g" \
           -e "s#$B#<build>#g" -e "s#$S#<source>#g" \
           -e "s#\(Host system: \)[^\\]*#\1QNX 8.0 $QNX_CPU (cross-built)#" \
           -e "s#\(Configured on: \)[^\\]*#\1(not recorded)#" "$f" || return 1
  done
  cmake --build "$B" -j"$(nproc)"
}
