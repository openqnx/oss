VERSION=1.51.0-1; GIT=https://github.com/luvit/luv.git; REF=1.51.0-1
DEPS="libuv luajit"
BUILDSYS=cmake
# libuv bindings for Lua -- the event loop under Neovim's Lua API.  Built as a
# shared library against the staged libuv and LuaJIT rather than its bundled
# submodule copies, so there is exactly one libuv in the tree.
CMAKE_ARGS_EXTRA=( -DBUILD_MODULE=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF
                   -DWITH_SHARED_LIBUV=ON -DWITH_LUA_ENGINE=LuaJIT
                   -DLUA_BUILD_TYPE=System
                   -DLIBUV_INCLUDE_DIR="$STAGE$PREFIX/include"
                   -DLIBUV_LIBRARIES="$STAGE$PREFIX/lib/libuv.so"
                   -DLUAJIT_INCLUDE_DIR="$STAGE$PREFIX/include/luajit-2.1"
                   -DLUAJIT_LIBRARIES="$STAGE$PREFIX/lib/libluajit-5.1.so" )
