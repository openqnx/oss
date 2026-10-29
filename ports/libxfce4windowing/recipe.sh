VERSION=4.20.0
# The release tarball, not the git tag: Xfce keeps its generated sources
# (*-alias.h, *-aliasdef.c, the .ui C sources) out of git and behind
# "if MAINTAINER_MODE", and regenerating them needs xdt-csource, which
# xfce4-dev-tools 4.20 no longer ships. The tarball has them all.
TARBALL=https://archive.xfce.org/src/xfce/libxfce4windowing/4.20/libxfce4windowing-4.20.0.tar.bz2
SHA256=56f29b1d79606fb00a12c83ef4ece12877d2b22bf1acaaff89537fbe8e939f68
DEPS="glib gtk3 wayland wayland-protocols"
BUILDSYS=autotools
# configure looks for wayland-scanner on PATH and finds nothing useful in a
# cross build; point it at the build-machine binary from hosttools/, which is
# the one that has to run.
CONF_ENV=( WAYLAND_SCANNER="$WS/hosttools/bin/wayland-scanner" )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-linker-opts --disable-x11 --enable-wayland )
