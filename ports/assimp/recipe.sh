# assimp: Open Asset Import Library -- loads 3D model formats (COLLADA, STL,
# OBJ, glTF, FBX, ...) into one scene structure. Needed by RViz (ros2-rviz)
# for robot meshes.
# Built against this tree's zlib (ASSIMP_BUILD_ZLIB off) rather than its copy.
# ASSIMP_WARNINGS_AS_ERRORS off: GCC 12's -Wstringop-truncation fires on
# assimp's own strncpy calls (EmbedTexturesProcess.cpp), which are bounded
# correctly. The `assimp` command-line tool is built too (info, export).
VERSION=6.0.2; GIT=https://github.com/assimp/assimp.git; REF=v6.0.2
DEPS="zlib"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DASSIMP_BUILD_TESTS=OFF -DASSIMP_BUILD_SAMPLES=OFF
                   -DASSIMP_BUILD_ASSIMP_TOOLS=ON -DASSIMP_BUILD_ZLIB=OFF
                   -DASSIMP_WARNINGS_AS_ERRORS=OFF -DASSIMP_INSTALL_PDB=OFF
                   -DCMAKE_INSTALL_LIBDIR=lib )
