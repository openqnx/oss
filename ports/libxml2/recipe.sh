VERSION=2.13.5; GIT=https://gitlab.gnome.org/GNOME/libxml2.git; REF=v2.13.5
DEPS="zlib xz python3"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DLIBXML2_WITH_PYTHON=ON
                   -DLIBXML2_WITH_TESTS=OFF -DLIBXML2_WITH_PROGRAMS=OFF
                   -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_LZMA=ON -DLIBXML2_WITH_ZLIB=ON
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
# Python bindings (libxml2.py + libxml2mod.so) for the target's python3, which
# itstool imports. The generator runs on the build host's Python of the same
# version; headers and library come from staging. They install into
# site-packages; the module links no libpython (patches/0001), so libxml2
# itself does not depend on python3.
_pv=3.14
CMAKE_ARGS_EXTRA+=( -DPython_EXECUTABLE="$WS/hosttools/bin/python$_pv"
                    -DPython_INCLUDE_DIR="$STAGE$PREFIX/include/python$_pv"
                    -DPython_LIBRARY="$STAGE$PREFIX/lib/libpython$_pv.so"
                    -DLIBXML2_PYTHON_INSTALL_DIR="$PREFIX/lib/python$_pv/site-packages" )
