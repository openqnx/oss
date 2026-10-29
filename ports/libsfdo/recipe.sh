# libsfdo: freedesktop.org desktop entry, icon theme and base directory
# specifications (labwc's menus and icons).
VERSION=0.1.4; GIT=https://gitlab.freedesktop.org/vyivel/libsfdo.git; REF=v0.1.4
DEPS=""
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dexamples=false -Dtests=false )
