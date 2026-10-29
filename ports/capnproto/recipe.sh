VERSION=1.0.2; GIT=https://github.com/capnproto/capnproto.git; REF=v1.0.2
DEPS="zlib openssl"
# Cap'n Proto names its C++ sources *.c++. qcc/q++ do not recognise that
# extension and silently produce no object at all (the link then fails with
# "cannot find .../array.c++.o"). files/qnx-cxx-source-language.cmake makes
# the language explicit in the compile rule only, so link lines are unchanged.
CMAKE_ARGS_EXTRA=( -DCMAKE_PROJECT_INCLUDE="$RECIPE_DIR/capnproto/files/qnx-cxx-source-language.cmake" -DBUILD_SHARED_LIBS=ON -DWITH_OPENSSL=ON -DWITH_ZLIB=ON -DWITH_FIBERS=OFF )
