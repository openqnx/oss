# pygame: Python game library on SDL2 -- surfaces, drawing, image load/save
# (SDL2_image), fonts (SDL2_ttf, FreeType), sound (SDL2_mixer), events,
# joysticks, and Wayland windows through SDL2's Wayland backend.
# The QNX build configuration is qnx-ports' (patches/0001), adapted for a
# cross build (0002). Built from the commit qnx-ports ported (pygame main
# after 2.6.1, which has the Python 3.12+ fixes 2.6.1 lacks); the Cython
# modules (_sdl2, pypm, _sprite) are generated from it on the build machine.
# Without portmidi (not ported) pygame.midi is left out; there is no camera
# module (Linux V4L2).
VERSION=2.6.1_git20251005; GIT=https://github.com/pygame/pygame.git; COMMIT=85fda3f719d437cf27106afae8c890e6b88ba5f5
DEPS="python3 numpy sdl2 sdl2-image sdl2-ttf sdl2-mixer freetype libpng libjpeg-turbo"
# numpy is optional at run time (pygame.surfarray, sndarray) but built against.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
# The wheel's .data/ (share/ files, scripts, headers) goes where pip puts it.
source "$WS/scripts/lib/pywheel-data.sh"
do_configure() {
  qnx_py_venv "setuptools>=65" "Cython>=3.0,<3.2" || return 1
  # The SDL2 headers: the sdl2 component's own (its package root), with the
  # SDL_image/ttf/mixer headers next to them, in a directory of the build's.
  # Staging can hold sdl2-compat's copies of the same files instead (a newer
  # version number): pygame compares the SDL version it was compiled with
  # against the library at import and refuses to run on an older one ("SDL
  # downgrade"), so it must be built against the sdl2 it depends on.
  local sdl="$WS/pkgroot/$QNX_ARCH/sdl2$PREFIX" h
  [ -f "$sdl/include/SDL2/SDL_version.h" ] || { echo "sdl2 package root missing: $sdl"; return 1; }
  rm -rf "$B/sdl2inc" && mkdir -p "$B/sdl2inc/SDL2" && cp -a "$sdl/include/SDL2/." "$B/sdl2inc/SDL2/" || return 1
  for h in SDL_image.h SDL_ttf.h SDL_mixer.h; do cp "$STAGE$PREFIX/include/SDL2/$h" "$B/sdl2inc/SDL2/" || return 1; done
  # sdl2-config for the configuration, answering in the three-call form
  # config_qnx.py expects (pkg-config fails on sdl2.pc's Requires.private
  # alsa, which the SDP provides without a .pc file).
  local v; v=$(sed -n 's/^#define SDL_\(MAJOR_VERSION\|MINOR_VERSION\|PATCHLEVEL\) *\([0-9]*\).*/\2/p' "$sdl/include/SDL2/SDL_version.h" | paste -sd.) || return 1
  cat > "$B/sdl2-config" <<S
#!/bin/sh
case "\$1" in
  --version) echo $v ;;
  --cflags) echo -I$B/sdl2inc/SDL2 -I$STAGE$PREFIX/include -D_REENTRANT ;;
  --libs) echo -L$sdl/lib -L$STAGE$PREFIX/lib -lSDL2 ;;
esac
S
  chmod +x "$B/sdl2-config"
  rm -f "$S/Setup"
}
# LOCALBASE: where config_qnx.py looks for headers and libraries (0002 stops
# it adding the build machine's /usr and /usr/local when cross compiling).
do_build() {
  rm -rf "$S/build"   # setuptools' build tree is in the source: no stale objects
  LOCALBASE="$STAGE$PREFIX" SDL_CONFIG="$B/sdl2-config" PORTMIDI_INC_PORTTIME=0 \
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" qnx_py_wheel
}
do_install() { qnx_py_install && qnx_py_install_data; }
