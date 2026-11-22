VERSION=0.10.1; GIT=https://github.com/wmww/gtk-layer-shell.git; REF=v0.10.1
DEPS="glib gtk3 wayland wayland-protocols"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=false -Dexamples=false -Ddocs=false
                   -Dintrospection=false -Dvapi=false )
