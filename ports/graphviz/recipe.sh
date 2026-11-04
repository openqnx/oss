# graphviz: graph layout and drawing -- dot, neato, circo, fdp, sfdp, twopi,
# osage, patchwork and the graph tools (gvpr, gc, acyclic, tred, ccomps ...),
# with libcgraph/libgvc for programs (doxygen draws its diagrams with dot).
VERSION=13.1.2; GIT=https://gitlab.com/graphviz/graphviz.git; REF=13.1.2
DEPS="gd cairo pango expat zlib libtool freetype fontconfig libwebp poppler glib libx11 libxrender"
BUILDSYS=cmake
# Renderers: cairo/pango (PNG, SVG, PDF, PS), gd (GIF, JPEG, ...), webp, PDF
# input (poppler), the X11 window output (-Txlib, Xwayland), and
# the core ones (SVG, dot, json, plain). Plugins are loaded with libltdl
# (libtool) from lib/graphviz, listed in its config file: `dot -c` writes that
# file on the target -- the apk trigger (apk.trigger) does it whenever a
# package changes lib/graphviz; after an oss-get install, run `dot -c` once.
# No GUIs (smyrna, gvedit: GTK2/Qt+OpenGL), no GDK plugin, no language
# bindings (SWIG, Tcl ...). Python runs the build's generator scripts.
APK_TRIGGERS="/usr/local/lib/graphviz"
CMAKE_ARGS_EXTRA=( -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=ON -DENABLE_LTDL=ON -DWITH_EXPAT=ON -DWITH_ZLIB=ON
                   -DWITH_GVEDIT=OFF -DWITH_SMYRNA=OFF -DWITH_GDK=OFF -Dwith_cxx_api=OFF
                   -DENABLE_TCL=OFF -DENABLE_SWIG=OFF -DENABLE_SHARP=OFF -DENABLE_D=OFF -DENABLE_GO=OFF
                   -DENABLE_GUILE=OFF -DENABLE_JAVA=OFF -DENABLE_JAVASCRIPT=OFF -DENABLE_LUA=OFF
                   -DENABLE_PERL=OFF -DENABLE_PHP=OFF -DENABLE_PYTHON=OFF -DENABLE_R=OFF -DENABLE_RUBY=OFF
                   -DPython3_EXECUTABLE=/usr/bin/python3
                   -DCMAKE_DISABLE_FIND_PACKAGE_GTK2=ON -DCMAKE_DISABLE_FIND_PACKAGE_GLUT=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_GTS=ON -DCMAKE_DISABLE_FIND_PACKAGE_ANN=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_DevIL=ON -DCMAKE_DISABLE_FIND_PACKAGE_AA=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_GS=ON )

# libgvc reaches libpathplan and libxdot through a static helper library, so
# CMake does not pass their build directories to the linker as -rpath-link,
# and the cross linker (which does not follow libgvc's DT_RPATH, as a native
# one would) cannot resolve libgvc's references when it links dot, gvpr ...:
# "libgvc.so: undefined reference to `Pshortestpath'". Name every library
# directory of the build tree for it (LDFLAGS from the environment goes in
# front of the toolchain's linker flags). Otherwise qnxbuild's own configure.
do_configure() {
  local rl="" d
  for d in "$S"/lib/*/; do d=${d%/}; rl="$rl${rl:+:}$B/lib/${d##*/}"; done
  LDFLAGS="-Wl,-rpath-link,$rl" cmake -S "$S" -B "$B" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$arch.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 "${CMAKE_ARGS_EXTRA[@]}"
}
