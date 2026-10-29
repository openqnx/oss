VERSION=4.4.1; GIT=https://github.com/godotengine/godot.git; REF=4.4.1-stable
# Godot 4.4.1 with qnx-ports' QNX support (patches/: the godot-QNX fork's
# commits on 4.4.1-stable -- a "qnx" platform on Screen/EGL/GLES3, QNX audio
# and input). Built here: the QNX export templates (template_release,
# template_debug), i.e. the runtime that runs Godot games on a QNX target,
# cross compiled with the SDP toolchain. The QNX Everywhere image also has the
# Godot editor; that one is built natively on QNX with clang (linuxbsd/Wayland
# platform) and is not cross-buildable this way.
# SCons options follow QNX's aports (extra/godot/APKBUILD), using this tree's
# libraries instead of the bundled copies.
DEPS="freetype harfbuzz graphite2 libogg libtheora libpng libvorbis libwebp opus pcre2 zlib zstd brotli libxkbcommon"
BUILDSYS=custom
_gd() {
  cd "$S" && PKG_CONFIG_LIBDIR="$PKG_CONFIG_LIBDIR" PKG_CONFIG_SYSROOT_DIR="$PKG_CONFIG_SYSROOT_DIR" \
  "$BUILD_DIR/host/scons-venv/bin/python" -m SCons -j"$(nproc)" platform=qnx arch="$([ "$QNX_CPU" = aarch64 ] && echo arm64 || echo x86_64)" \
    module_camera_enabled=false builtin_brotli=false builtin_freetype=false builtin_graphite=false builtin_harfbuzz=false \
    builtin_libogg=false builtin_libtheora=false builtin_libpng=false builtin_libvorbis=false builtin_libwebp=false \
    builtin_opus=false builtin_pcre2=false builtin_zlib=false builtin_zstd=false engine_update_check=false execinfo=false \
    lto=none production=true system_certs_path=/usr/local/ssl/cert.pem use_sowrap=false use_static_cpp=false \
    module_denoise_enabled=false import_env_vars=PKG_CONFIG_LIBDIR,PKG_CONFIG_SYSROOT_DIR,PKG_CONFIG_PATH "$@"
}
# import_env_vars: SCons runs pkg-config (ParseConfig) in its own minimal
# environment, which then answered from the build machine's .pc files --
# host freetype2 flags, no theora at all. Hand it the tree's pkg-config setup.
do_build() {
  # SCons in a venv of its own: the build machine's Python may refuse a pip
  # install outside one ("externally-managed-environment").
  local v="$BUILD_DIR/host/scons-venv"
  [ -x "$v/bin/scons" ] || { python3 -m venv "$v" && "$v/bin/pip" install -q scons; } || return 1
  _gd target=template_release && _gd target=template_debug
}
do_install() {
  install -d "$STAGE$PREFIX/bin" && \
  install -m755 "$S"/bin/godot.qnx.template_release.* "$STAGE$PREFIX/bin/godot-template-release-qnxscreen" && \
  install -m755 "$S"/bin/godot.qnx.template_debug.* "$STAGE$PREFIX/bin/godot-template-debug-qnxscreen" && \
  rm -f "$S"/bin/godot.qnx.*
}
