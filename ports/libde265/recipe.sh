VERSION=1.0.15; GIT=https://github.com/strukturag/libde265.git; REF=v1.0.15
BUILDSYS=cmake
# The other half of the HEVC pair: x265 (already staged) encodes, libde265
# decodes.  No SDL/Qt example programs -- they pull in a display stack the
# library itself does not need.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DENABLE_SDL=OFF
                   -DENABLE_DECODER=ON -DENABLE_ENCODER=OFF )

# Note on the soname: upstream's CMake build sets no VERSION/SOVERSION on the
# target, so it installs an unversioned libde265.so with SONAME "libde265.so"
# -- where its autotools build produces libde265.so.0.  That is upstream's
# behaviour, not something this recipe chose, and it is left alone rather than
# invented here: a soname this tree makes up would differ from every other
# distribution's.  scripts/check-devlinks only scans lib*.so.* so it does not
# flag this; the library exports 773 symbols and links correctly.
