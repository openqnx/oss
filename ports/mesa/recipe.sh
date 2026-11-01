# mesa: the Mesa 3D graphics library as a software renderer -- desktop OpenGL
# (4.6 core), OpenGL ES 1/2/3.2 and EGL on llvmpipe (LLVM-JIT rasterizer,
# from the llvm component) and softpipe, plus lavapipe (software Vulkan 1.4).
# EGL platforms: surfaceless (pbuffers, off-screen rendering) and wayland
# (wl_shm buffers: any Wayland compositor, labwc included). The SDP's own
# libEGL/libGLESv2 (for QNX Screen, the GPU drivers) keep /usr/lib: Mesa goes
# in a prefix of its own, /usr/local/opt/mesa, nothing linked into bin/ or
# lib/. Its libraries are renamed (libEGL_mesa.so.1, libGLESv2_mesa.so.2,
# libGLESv1_CM_mesa.so.1): the SDP's are libEGL.so.1 and libGLESv2.so.1, and
# QNX's loader takes LD_LIBRARY_PATH before RPATH, so one soname in two
# places would hand the compositor (wlroots, on the SDP's EGL) Mesa's
# library or a Mesa client the SDP's. Applications link Mesa through its
# pkg-config files (egl.pc, glesv2.pc in opt/mesa/lib/pkgconfig), or load
# libEGL_mesa.so.1 by path (SDL: SDL_VIDEO_EGL_DRIVER).
VERSION=26.2.6
TARBALL=https://archive.mesa3d.org/mesa-26.2.6.tar.xz
SHA256=b9436b3e1a8be76fbd4823dc884f90596ab7c6ac5e1867a305e66ffccfb31a92
DEPS="llvm wayland wayland-protocols zlib zstd expat"
BUILDSYS=meson
OPT="$PREFIX/opt/mesa"
# No GPU drivers and no DRM/KMS: QNX has no DRM kernel interface, so libdrm,
# GBM, the EGL "drm" platform and kms_swrast are off (Mesa's meson.build makes
# that choice itself for a system it does not list as having KMS). No GLX/X11
# (the X server here is Xwayland; X clients keep using what they use), no
# glvnd (not ported). Valgrind/libunwind/lmsensors annotations off.
# Run path: the prefix's lib/ and /usr/local/lib (libLLVM.so.22.1, libz,
# libzstd, libexpat, libwayland-*), absolute: QNX's loader does not expand
# $ORIGIN.
MESA_PLATFORMS=wayland
MESON_ARGS_EXTRA=( --prefix="$OPT" -Dlibdir=lib -Dbuildtype=release -Db_ndebug=true
  -Dplatforms=$MESA_PLATFORMS -Degl-native-platform=surfaceless
  -Dgallium-drivers=llvmpipe,softpipe -Dvulkan-drivers=swrast
  -Degl-lib-suffix=_mesa -Dgles-lib-suffix=_mesa
  -Degl=enabled -Dgles1=enabled -Dgles2=enabled -Dopengl=true -Dglx=disabled -Dgbm=disabled -Dglvnd=disabled
  -Dllvm=enabled -Dshared-llvm=enabled -Ddraw-use-llvm=true
  -Dgallium-va=disabled -Dgallium-rusticl=false -Dteflon=false
  -Dvalgrind=disabled -Dlibunwind=disabled -Dlmsensors=disabled -Dxlib-lease=disabled
  -Dzstd=enabled -Dxmlconfig=enabled -Dexpat=enabled -Dshader-cache=enabled
  -Dbuild-tests=false -Dtools=[] -Dhtml-docs=disabled -Dvulkan-layers=[]
  -Dcpp_rtti=true )

# Build-machine Python modules Mesa's generators need, which neither the host
# nor hosttools/ has: Mako and MarkupSafe (code templates), PyYAML, packaging,
# and a Meson new enough for Mesa (>= 1.4; the host's is 1.3). All pure
# Python, pinned by checksum, unpacked into build/host/mesa-python once.
MESA_PY="$BUILD_DIR/host/mesa-python"
_mesa_py() {
  [ -f "$MESA_PY/.done" ] && return 0
  rm -rf "$MESA_PY"; mkdir -p "$MESA_PY/src" "$WS/dist"
  local e u f sum
  for e in \
    "https://files.pythonhosted.org/packages/87/fb/99f81ac72ae23375f22b7afdb7642aba97c00a713c217124420147681a2f/mako-1.3.10-py3-none-any.whl baef24a52fc4fc514a0887ac600f9f1cff3d82c61d4d700a1fa84d597b88db59" \
    "https://files.pythonhosted.org/packages/7e/99/7690b6d4034fffd95959cbe0c02de8deb3098cc577c67bb6a24fe5d7caa7/markupsafe-3.0.3.tar.gz 722695808f4b6457b320fdc131280796bdceb04ab50fe1795cd540799ebe1698" \
    "https://files.pythonhosted.org/packages/05/8e/961c0007c59b8dd7729d542c61a4d537767a59645b82a0b521206e1e25c2/pyyaml-6.0.3.tar.gz d76623373421df22fb4cf8817020cbb7ef15c725b9d5e45f17e189bfc384190f" \
    "https://files.pythonhosted.org/packages/20/12/38679034af332785aac8774540895e234f4d07f7545804097de4b666afd8/packaging-25.0-py3-none-any.whl 29572ef2b1f17581046b3a2227d5c611fb25ec70ca1ba8554b24b0e69331a484" \
    "https://files.pythonhosted.org/packages/32/4f/c398c6f06ece1c6c246e008d5dac3824c98f54d3eb3d8014f4910afd6d48/meson-1.10.0-py3-none-any.whl 4b27aafce281e652dcb437b28007457411245d975c48b5db3a797d3e93ae1585"; do
    u=${e% *}; sum=${e#* }; f="$WS/dist/${u##*/}"
    [ -f "$f" ] || curl -fsSL --retry 3 -o "$f" "$u" || return 1
    echo "$sum  $f" | sha256sum -c --status || { echo "!! checksum mismatch: $f"; rm -f "$f"; return 1; }
    case "$f" in
      *.whl) python3 -m zipfile -e "$f" "$MESA_PY" || return 1 ;;
      *) tar xf "$f" -C "$MESA_PY/src" || return 1 ;;
    esac
  done
  # The two source archives: their pure-Python packages, without the C
  # speedups (both fall back to Python when those are absent).
  cp -r "$MESA_PY"/src/markupsafe-*/src/markupsafe "$MESA_PY"/src/pyyaml-*/lib/yaml "$MESA_PY/" || return 1
  # A real script (Meson writes its own path into build.ninja), which finds
  # these modules next to itself.
  printf '#!/usr/bin/env python3\nimport os, sys\nsys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))\nfrom mesonbuild.mesonmain import main\nsys.exit(main())\n' > "$MESA_PY/meson"
  chmod 755 "$MESA_PY/meson"; touch "$MESA_PY/.done"
}

# LLVM: Mesa asks llvm-config, and the staged one is a QNX program. This
# stand-in answers the questions Meson asks it from the staged LLVM (its
# version, components, flags, the shared libLLVM).
_mesa_llvm_config() {
  local L="$STAGE$PREFIX" v comps
  v=$(sed -n 's/^set(LLVM_PACKAGE_VERSION \(.*\))/\1/p' "$L/lib/cmake/llvm/LLVMConfigVersion.cmake" | head -1)
  [ -n "$v" ] || v=$(sed -n 's/.*LLVM_VERSION_STRING "\(.*\)".*/\1/p' "$L/include/llvm/Config/llvm-config.h")
  comps=$(cd "$L/lib" && ls libLLVM*.a | sed 's/^libLLVM\(.*\)\.a$/\1/' | tr 'A-Z' 'a-z' | tr '\n' ' ')
  comps="$comps all all-targets native nativecodegen engine aarch64 x86 webassembly"
  cat > "$B/llvm-config" <<EOF
#!/bin/sh
# Generated by ports/mesa/recipe.sh: llvm-config for the staged QNX LLVM.
out=""
for a in "\$@"; do case "\$a" in
  --version) out="\$out $v" ;;
  --components) out="\$out $comps" ;;
  --shared-mode) out="\$out shared" ;;
  --has-rtti) out="\$out YES" ;;
  --libdir) out="\$out $L/lib" ;;
  --includedir) out="\$out $L/include" ;;
  --prefix) out="\$out $L" ;;
  --bindir) out="\$out $L/bin" ;;
  --cppflags) out="\$out -I$L/include -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS" ;;
  --cxxflags) out="\$out -I$L/include -std=c++17 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS" ;;
  --cflags) out="\$out -I$L/include -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS" ;;
  --ldflags) out="\$out -L$L/lib" ;;
  --libs) out="\$out -lLLVM" ;;
  --libfiles) out="\$out $L/lib/libLLVM.so" ;;
  --system-libs|--link-shared|--link-static) : ;;
  --targets-built) out="\$out AArch64 X86 WebAssembly" ;;
  --host-target) out="\$out $([ "$QNX_CPU" = aarch64 ] && echo aarch64-unknown-qnx || echo x86_64-pc-qnx)" ;;
  --assertion-mode) out="\$out OFF" ;;
  --build-mode) out="\$out Release" ;;
  -*) echo "llvm-config: unsupported \$a" >&2; exit 1 ;;
  *) : ;;
esac; done
echo \$out
EOF
  chmod 755 "$B/llvm-config"
}

do_configure() {
  _mesa_py || return 1
  _mesa_llvm_config || return 1
  local cf="$B/cross.ini" nf=()
  # The cross file plus the stand-in llvm-config and the run path (in the
  # cross file's link arguments: a -Dc_link_args would replace them), and
  # libregex: driconf's application matching uses regcomp(), which QNX keeps
  # out of libc.
  sed -e "s#^\[binaries\]#[binaries]\nllvm-config = '$B/llvm-config'#" \
      -e "s#^\(c\(pp\)\?_link_args *= *\[\)#\1'-Wl,-rpath,$OPT/lib:$PREFIX/lib', #" \
      -e "s#'-lfsnotify'\]#'-lfsnotify', '-lregex']#" \
      "$QNX_MESON_DIR/meson-$arch.ini" > "$cf"
  [ -f "$QNX_MESON_DIR/meson-native.ini" ] && nf=( --native-file "$QNX_MESON_DIR/meson-native.ini" )
  PYTHONPATH="$MESA_PY" "$MESA_PY/meson" setup "$B" "$S" --cross-file "$cf" "${nf[@]}" "${MESON_ARGS_EXTRA[@]}"
}
# LLVM-linked C++: moderate parallelism (other builds share the machine).
do_build() { PYTHONPATH="$MESA_PY" ninja -C "$B" -j6; }
# lib/dropin/: the usual names (libEGL.so.1, libGLESv2.so.2, libGLESv1_CM.so.1)
# as links to Mesa's, for programs that dlopen() them (SDL, GLFW, libepoxy):
# LD_LIBRARY_PATH=/usr/local/opt/mesa/lib/dropin for that one program
# chooses Mesa. Never on a system-wide path (the compositor needs the SDP's).
do_install() {
  PYTHONPATH="$MESA_PY" DESTDIR="$STAGE" "$MESA_PY/meson" install -C "$B" --no-rebuild
  local d="$STAGE$OPT/lib/dropin"; mkdir -p "$d"
  ln -sf ../libEGL_mesa.so.1 "$d/libEGL.so.1"; ln -sf ../libGLESv2_mesa.so.2 "$d/libGLESv2.so.2"
  ln -sf ../libGLESv1_CM_mesa.so.1 "$d/libGLESv1_CM.so.1"
}
