# libportal: XDG desktop portal client library (GTK3 flavour).
VERSION=0.8.1; GIT=https://github.com/flatpak/libportal.git; REF=0.8.1
DEPS="glib gtk3"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dbackend-gtk3=enabled -Dbackend-gtk4=disabled -Dbackend-qt5=disabled -Dbackend-qt6=disabled
                   -Dintrospection=false -Dvapi=false -Ddocs=false -Dtests=false )
