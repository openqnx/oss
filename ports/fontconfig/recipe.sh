VERSION=2.15.0; GIT=https://gitlab.freedesktop.org/fontconfig/fontconfig.git; REF=2.15.0
DEPS="freetype expat"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Ddoc=disabled -Dtests=disabled -Dtools=disabled
                   -Dnls=enabled -Dcache-build=disabled -Diconv=disabled )
