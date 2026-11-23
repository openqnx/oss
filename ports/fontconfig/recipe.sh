VERSION=2.18.4; GIT=https://gitlab.freedesktop.org/fontconfig/fontconfig.git; REF=2.18.4
DEPS="freetype expat"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Ddoc=disabled -Dtests=disabled -Dtools=disabled
                   -Dnls=enabled -Dcache-build=disabled -Diconv=disabled )
# fontconfig 2.18 requires meson >= 1.11, newer than the build image's default
HOSTTOOLS_NEW=1
