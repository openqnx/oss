VERSION=0.9.0; GIT=https://github.com/wmww/gtk-layer-shell.git; REF=v0.9.0
DEPS="glib gtk3 wayland wayland-protocols"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=false -Dexamples=false -Ddocs=false
                   -Dintrospection=false -Dvapi=false )
