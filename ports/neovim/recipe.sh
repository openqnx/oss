VERSION=0.12.3; GIT=https://github.com/neovim/neovim.git; REF=v0.12.3
# Neovim with the QNX changes of QNX's aports (patches/), using this tree's
# libraries (USE_BUNDLED=OFF) as the QNX Everywhere image does.
#
# Neovim generates sources with Lua scripts while it builds, run by the host
# LuaJIT with "nlua0" -- a module (LPeg, mpack, ...) that must therefore be
# built for the *build host* as well. 0.12 takes a host-built one through
# NLUA0_HOST_PRG; without it, the cross build uses its own QNX nlua0 and the
# host LuaJIT cannot load it ("libsocket.so.4: cannot open shared object").
# So, on the build host: cmake.deps makes Neovim's bundled dependencies, then
# a native configure of Neovim builds the nlua0 target against them -- and a
# host nvim, which the build runs to generate the help tags (NVIM_HOST_PRG).
DEPS="luajit libuv utf8proc tree-sitter unibilium lua-lpeg luv libiconv gettext"
BUILDSYS=custom
_host() { env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u LDFLAGS -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH "$@"; }
do_configure() {
  local H="$BUILD_DIR/host/neovim-$VERSION-deps" N="$BUILD_DIR/host/neovim-$VERSION-nlua0"
  if [ ! -x "$H/usr/bin/luajit" ]; then
    _host cmake -S "$S/cmake.deps" -B "$H" -G Ninja -DUSE_BUNDLED=ON -DUSE_BUNDLED_GETTEXT=OFF -DUSE_BUNDLED_LIBICONV=OFF \
      && _host ninja -C "$H" || return 1
  fi
  if [ ! -f "$N/lib/libnlua0.so" ] || [ ! -x "$N/bin/nvim" ]; then
    _host cmake -S "$S" -B "$N" -G Ninja -DCMAKE_BUILD_TYPE=Release -DDEPS_PREFIX="$H/usr" -DENABLE_TRANSLATIONS=OFF \
      && _host ninja -C "$N" nlua0 nvim_bin || return 1
  fi
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCI_BUILD=OFF -DENABLE_TRANSLATIONS=OFF \
    -DLUA_PRG="$H/usr/bin/luajit" -DLUA_GEN_PRG="$H/usr/bin/luajit" -DNLUA0_HOST_PRG="$N/lib/libnlua0.so" -DNVIM_HOST_PRG="$N/bin/nvim" \
    -DLPEG_LIBRARY="$STAGE$PREFIX/lib/lua/5.1/lpeg.so"
}
do_build() { cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
