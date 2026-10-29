VERSION=0.5.9; GIT=https://github.com/libproxy/libproxy.git; REF=0.5.9
DEPS="glib curl"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=false -Ddocs=false -Dintrospection=false -Dvapi=false
                   -Dconfig-gnome=false -Dconfig-kde=false -Dconfig-osx=false
                   -Dconfig-windows=false -Dpacrunner-duktape=false
                   -Dcurl=true )
