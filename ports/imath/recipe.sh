# imath: C++ vector, matrix, quaternion and half-float math library (the one
# OpenEXR is built on). Ported by request; see ports/imath/README.md.
# Shared library only; no Python bindings (PyImath) -- they need boost-python.
VERSION=3.2.2; GIT=https://github.com/AcademySoftwareFoundation/Imath.git; REF=v3.2.2
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=ON -DPYTHON=OFF )
