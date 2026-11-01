VERSION=3.1.0; GIT=https://github.com/pybind/pybind11.git; REF=v3.1.0
# Header-only: the headers and the CMake package (pybind11Config.cmake,
# pybind11_add_module) for building extension modules against the ported
# python3. Configuring does not need a Python: PYBIND11_NOPYTHON skips the
# interpreter probe, which in a cross build would find the build machine's.
# prefix_for_pc_file: with CMake >= 3.20 upstream derives the .pc prefix with
# cmake_path(RELATIVE_PATH) the wrong way round and writes an empty prefix=
# (so -I/include); name it.
CMAKE_ARGS_EXTRA=( -DPYBIND11_TEST=OFF -DPYBIND11_INSTALL=ON -DPYBIND11_NOPYTHON=ON -Dprefix_for_pc_file="$PREFIX" )
