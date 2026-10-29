VERSION=2.32.10; GIT=https://github.com/libsdl-org/SDL.git; REF=release-2.32.10
# sdl2-compat installs the same libSDL2-2.0 and headers: install one or the other.
CONFLICTS="sdl2-compat"
DEPS="wayland libxkbcommon"
CMAKE_ARGS_EXTRA=( -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST=OFF
                   -DSDL_X11=OFF -DSDL_WAYLAND=ON -DSDL_WAYLAND_SHARED=OFF
                   -DSDL_KMSDRM=OFF -DSDL_RPI=OFF -DSDL_OPENGL=OFF -DSDL_OPENGLES=OFF
                   -DSDL_VULKAN=OFF -DSDL_PULSEAUDIO=OFF -DSDL_ALSA=ON -DSDL_ALSA_SHARED=OFF
                   -DSDL_JACK=OFF -DSDL_PIPEWIRE=OFF -DSDL_SNDIO=OFF
                   -DSDL_LIBUDEV=OFF -DSDL_DBUS=OFF -DSDL_IBUS=OFF )
