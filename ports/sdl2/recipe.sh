VERSION=2.32.10; GIT=https://github.com/libsdl-org/SDL.git; REF=release-2.32.10
# sdl2-compat installs the same libSDL2-2.0 and headers: install one or the other.
CONFLICTS="sdl2-compat"
# SDL_iconv links the libiconv built here (libiconv.so.2).
DEPS="libiconv"
# Video: SDL's QNX Screen driver ("qnx"), with OpenGL ES 2 through the SDP's
# EGL/GLESv2 -- the way the SDP's own gles2-gears draws, on Screen directly,
# with or without a Wayland compositor running. CheckQNXScreen needs EGL
# (SDL_OPENGLES=ON finds the SDP's <EGL/egl.h>, <GLES2/gl2.h>); libSDL2 links
# libscreen.so.1 and libEGL.so.1 and loads libGLESv2.so.1 at run time (QNX
# OS libraries: the screen images have them in /system/lib, target-bootstrap
# copies them to the vanilla ones). Patch 0002 makes the 2017 driver usable
# (display mode, pointer/close/focus events, more keys and text input,
# resizable framebuffers, GLES entry points). Wayland stays off: SDL2's
# Wayland driver presents only through EGL on the wl_display, which the SDP's
# Screen-only EGL cannot do, and Mesa's EGL is renamed and kept out of
# library paths (ports/mesa/README.md); built, it would be picked first under
# labwc and fail at window creation. SDL_VIDEODRIVER=offscreen|dummy for
# headless use; with no Screen running the qnx driver steps aside.
CMAKE_ARGS_EXTRA=( -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST=OFF
                   -DSDL_X11=OFF -DSDL_WAYLAND=OFF
                   -DSDL_KMSDRM=OFF -DSDL_RPI=OFF -DSDL_OPENGL=OFF -DSDL_OPENGLES=ON
                   -DSDL_VULKAN=OFF -DSDL_PULSEAUDIO=OFF -DSDL_ALSA=ON -DSDL_ALSA_SHARED=OFF
                   -DSDL_JACK=OFF -DSDL_PIPEWIRE=OFF -DSDL_SNDIO=OFF
                   -DSDL_LIBUDEV=OFF -DSDL_DBUS=OFF -DSDL_IBUS=OFF )
