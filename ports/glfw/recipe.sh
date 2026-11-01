# GLFW: windows, input and OpenGL/Vulkan context setup for desktop programs.
# Platforms built: Wayland and the null platform (always built: headless, no
# display needed). X11 is off -- this tree has no X server to run it on.
# GLFW loads libwayland-client/-cursor/-egl and libxkbcommon at run time
# (dlopen); the Wayland and xkbcommon headers are needed to build it.
# Contexts: GLFW's EGL path is built and loads libEGL.so.1 at run time, but
# the SDP's EGL drives QNX Screen, not Wayland surfaces: on Wayland, windows
# without a client API (GLFW_NO_API, for Vulkan or software rendering) are
# what works.
VERSION=3.5.1; GIT=https://github.com/glfw/glfw.git; REF=3.5.1
DEPS="wayland libxkbcommon linux-headers-input-codes"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF
                   -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_X11=OFF -DGLFW_BUILD_WAYLAND=ON )
