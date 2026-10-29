VERSION=24.14.1; GIT=https://github.com/nodejs/node.git; REF=v24.14.1
# Node.js 24 with qnx-ports' QNX support. patches/0001: the qnx-ports/node
# fork's QNX commit (libuv, V8 and the build files learn QNX); 0002-0006:
# QNX's aports changes the QNX Everywhere image builds with. The shared
# libraries of this tree are used instead of the bundled copies, as aports does.
#
# Cross build: V8's build runs generator programs (mksnapshot, torque, ...) on
# the build host, so configure gets the host compiler for those (CC_host) and
# the QNX compiler for the target.
DEPS="libuv ada brotli zlib openssl c-ares nghttp2 simdjson simdutf sqlite zstd icu"
BUILDSYS=custom
# V8's gyp files test host_os for the host toolset (mksnapshot, torque run on
# the build machine). toolchain.gypi gives it a default of OS, but in a cross
# build that default is not in scope where v8.gyp reads it ("name 'host_os' is
# not defined"); name the build machine's OS explicitly.
# Node adds the include directory of every shared library it finds through
# pkg-config to *all* its targets -- V8's bundled abseil too, which then
# picked up the abseil component's headers (same lts_ namespace) instead of
# its own: "redefinition of absl::...::TrailingZeros"; googletest's the same.
# pkg-config therefore gets a sysroot whose include directory is the staging
# one without the headers of what Node and V8 bundle and do not take shared.
NODE_BUNDLED_HEADERS="absl gtest gmock fp16 fp16.h hwy json re2 fast_float cpu_features"
_node_sysroot() {
  local r="$B/node-sysroot" e
  rm -rf "$r" && mkdir -p "$r$PREFIX/include" && ln -s "$STAGE$PREFIX/lib" "$r$PREFIX/lib" || return 1
  for e in "$STAGE$PREFIX/include"/*; do
    case " $NODE_BUNDLED_HEADERS " in *" $(basename "$e") "*) continue ;; esac
    ln -s "$e" "$r$PREFIX/include/"
  done
  echo "$r"
}
# Node's host tools (node_js2c and friends) are linked with the same shared
# libraries as node itself -- for the build host, where most of them are not
# installed ("cannot find -lada", -lcares, -lnghttp2, -lsimdjson, -lsimdutf,
# -lsqlite3, -luv) -- or not in the version the staged headers are (ICU 76:
# "undefined reference to icu_76::UnicodeSet..."). Build those for the host from the same sources and
# versions as their own components (their recipes say which), into one
# prefix, and give it to the host link (LDFLAGS_host; rpath, so they run).
NODE_HOST_LIBS="ada c-ares nghttp2 simdjson simdutf sqlite libuv icu"
_host() { env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u CPPFLAGS -u LDFLAGS -u LIBS -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH "$@"; }
_node_hostlibs() {
  local H="$1" n
  [ -f "$H/.done" ] && return 0
  mkdir -p "$H/src" || return 1
  for n in $NODE_HOST_LIBS; do
    echo "-- host library for node's build tools: $n"
    ( GIT= REF= COMMIT= TARBALL= SHA256= CFLAGS_APPEND=; CONF_ARGS_EXTRA=(); source "$WS/ports/$n/recipe.sh"
      d="$H/src/$n"; rm -rf "$d"
      if [ -n "$GIT" ]; then git clone -q --depth 1 --branch "$REF" "$GIT" "$d"
      else t="$WS/dist/$(basename "$TARBALL")"
           { [ -f "$t" ] || curl -fsSL -o "$t" "$TARBALL"; } && echo "$SHA256  $t" | sha256sum -c --status && \
           mkdir -p "$d" && tar xf "$t" -C "$d" --strip-components=1
      fi || exit 1
      if [ -f "$d/CMakeLists.txt" ]; then
        _host cmake -S "$d" -B "$d/_b" -G Ninja --no-warn-unused-cli -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_INSTALL_PREFIX="$H" -DCMAKE_INSTALL_LIBDIR=lib -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF \
          -DENABLE_LIB_ONLY=ON -DADA_TESTING=OFF -DADA_TOOLS=OFF -DADA_BENCHMARKS=OFF -DSIMDJSON_DEVELOPER_MODE=OFF \
          -DSIMDUTF_TESTS=OFF -DSIMDUTF_TOOLS=OFF -DLIBUV_BUILD_TESTS=OFF -DCARES_BUILD_TOOLS=OFF && \
        _host ninja -C "$d/_b" install
      else
        local c="$d"; [ -x "$d/icu4c/source/configure" ] && c="$d/icu4c/source"   # ICU keeps it there
        # The component's own configure options and flags (sqlite: the session
        # extension node:sqlite needs), then the host prefix.
        ( cd "$c" && _host env CFLAGS="-O2 $CFLAGS_APPEND" ./configure "${CONF_ARGS_EXTRA[@]}" \
            --prefix="$H" --libdir="$H/lib" --disable-static && _host make -j"$(nproc)" install )
      fi ) || return 1
  done
  touch "$H/.done"
}
do_configure() {
  local R; R=$(_node_sysroot) || return 1
  local HL="$BUILD_DIR/host/node-$VERSION-libs"
  _node_hostlibs "$HL" || return 1
  # Node builds in the source tree (out/); the other architecture's objects
  # must not be reused.
  rm -rf "$S/out" && cd "$S" && PKG_CONFIG_SYSROOT_DIR="$R" GYP_DEFINES="host_os=linux" CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc" CXX="$QNX_HOST/usr/bin/$QNX_TRIPLE-g++" CC_host=gcc CXX_host=g++ \
  CFLAGS="-D_QNX_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DOPENSSL_NO_ENGINE -idirafter $STAGE$PREFIX/include" \
  CXXFLAGS="-D_QNX_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DOPENSSL_NO_ENGINE -idirafter $STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib" PKG_CONFIG_LIBDIR="$PKG_CONFIG_LIBDIR" \
  python3 configure.py --prefix="$PREFIX" --dest-os=qnx --dest-cpu="$([ "$QNX_CPU" = aarch64 ] && echo arm64 || echo x64)" \
    --cross-compiling --use-prefix-to-find-headers --shared-libuv --shared-ada --shared-brotli --shared-zlib \
    --shared-openssl --shared-cares --shared-nghttp2 --shared-simdjson --shared-simdutf --shared-sqlite --shared-zstd \
    --openssl-use-def-ca-store --with-intl=system-icu --without-corepack --without-npm \
    --shared-libuv-includes="$R$PREFIX/include" --shared-libuv-libpath="$STAGE$PREFIX/lib"
}
# The target compilers are the SDP's GCC driver (<triple>-gcc/g++) rather than
# qcc/q++: gyp's Make generator writes dependency files with -MMD -MF, which qcc
# misreads ("Can't specify ... -o and have multiple files"). It links the same
# runtime (libc++) as q++ -V..._cxx.
# Staged headers come in with -idirafter, after the include paths of the
# sources Node bundles: with -I, the abseil component's headers and the copy in
# deps/v8/third_party (the same lts_ namespace) were both included --
# "redefinition of ... absl::lts_20240722::container_internal::TrailingZeros".
# gyp's Make generator, not --ninja: in a cross build the inspector headers
# target exists for the host and the target toolset, both writing the same
# stamp, which ninja rejects ("multiple rules generate .../js_protocol.stamp").
# LDFLAGS_host is read when make runs (out/Makefile: LDFLAGS.host ?= ...).
# DT_RPATH (--disable-new-dtags), not DT_RUNPATH: only the former also covers
# the libraries those libraries load (libicuuc -> libicudata.so.76).
# V8's code generators are built first: their "run_" actions exist for the
# host and the target toolset with the same outputs, Make keeps the target
# copy, and under -j it ran before the host binary was linked (Error 127,
# "gen-regexp-special-case: No such file").
NODE_HOST_TOOLS="gen-regexp-special-case torque bytecode_builtins_list_generator"
do_build() { local HL="$BUILD_DIR/host/node-$VERSION-libs"
  LDFLAGS_host="-L$HL/lib -Wl,-rpath,$HL/lib -Wl,--disable-new-dtags" make -C "$S/out" BUILDTYPE=Release -j"$(nproc)" $NODE_HOST_TOOLS && \
  LDFLAGS_host="-L$HL/lib -Wl,-rpath,$HL/lib -Wl,--disable-new-dtags" make -C "$S/out" BUILDTYPE=Release -j"$(nproc)"; }
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$S/out/Release/node" "$STAGE$PREFIX/bin/" && \
  cd "$S" && python3 tools/install.py install --dest-dir "$STAGE" --prefix "$PREFIX" --headers-only; }
