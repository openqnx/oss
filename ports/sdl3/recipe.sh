VERSION=3.4.18; GIT=https://github.com/libsdl-org/SDL.git; REF=release-3.4.18
# Also linked at run time (found by scripts/gen-dependency-index): libiconv libxtst
DEPS="libx11 libxext libxcursor libxi libxfixes libxrandr libxrender libxscrnsaver libxkbcommon libiconv libxtst"
BUILDSYS=cmake
# X11, not Wayland -- the opposite of the SDL2 recipe here, and not a
# preference.  SDL3's Wayland check requires "wayland-egl" and "egl" in its
# pkg-config spec unconditionally (cmake/sdlchecks.cmake, CheckWayland), and
# there is no EGL in this tree: QNX has no Mesa, and its own EGL belongs to
# Screen, which is exactly the kind of SDP prebuilt this workspace must not
# link.  SDL3's X11 backend has no such requirement, and the full X11 client
# stack is staged -- so X11 is the backend that can actually be built from
# source here.  GL/GLES/Vulkan stay off for the same EGL reason; SDL3 keeps
# its software renderer, which is what runs.
CMAKE_ARGS_EXTRA=( -DSDL_SHARED=ON -DSDL_STATIC=OFF
                   -DSDL_TEST_LIBRARY=OFF -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF
                   -DSDL_X11=ON -DSDL_X11_SHARED=OFF -DSDL_WAYLAND=OFF
                   -DSDL_KMSDRM=OFF -DSDL_RPI=OFF
                   -DSDL_OPENGL=OFF -DSDL_OPENGLES=OFF -DSDL_VULKAN=OFF
                   -DSDL_PULSEAUDIO=OFF -DSDL_ALSA=ON -DSDL_ALSA_SHARED=OFF
                   -DSDL_JACK=OFF -DSDL_PIPEWIRE=OFF -DSDL_SNDIO=OFF
                   -DSDL_LIBUDEV=OFF -DSDL_DBUS=OFF -DSDL_IBUS=OFF )
