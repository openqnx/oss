VERSION=0.46.4; GIT=https://gitlab.freedesktop.org/pixman/pixman.git; REF=pixman-0.46.4
DEPS="libpng"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=disabled -Ddemos=disabled -Dgtk=disabled -Dlibpng=enabled )
