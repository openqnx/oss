VERSION=1.5.10; GIT=https://github.com/anholt/libepoxy.git; REF=1.5.10
DEPS="libx11"
BUILDSYS=meson
# EGL dispatch for QNX's Screen EGL, plus GLX dispatch.  GLX was off at first
# ("QNX has no X11"), but GTK3's X11 backend -- needed by Thunar -- includes
# <epoxy/glx.h> unconditionally.  Building it needs only the X11 headers:
# epoxy dlopens libGL on first use, and GTK asks through epoxy_has_glx(), which
# loads with exit_on_fail=false and simply answers false when there is no libGL
# (dispatch_glx.c).  No GL implementation is built or linked here.
MESON_ARGS_EXTRA=( -Dglx=yes -Degl=yes -Dx11=true -Dtests=false -Ddocs=false )
