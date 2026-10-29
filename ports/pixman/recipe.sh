VERSION=0.44.2; GIT=https://gitlab.freedesktop.org/pixman/pixman.git; REF=pixman-0.44.2
DEPS="libpng"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=disabled -Ddemos=disabled -Dgtk=disabled -Dlibpng=enabled )
