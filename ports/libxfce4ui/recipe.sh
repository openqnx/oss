VERSION=4.20.2
# The release tarball, not the git tag: Xfce keeps its generated sources
# (*-alias.h, *-aliasdef.c, the .ui C sources) out of git and behind
# "if MAINTAINER_MODE", and regenerating them needs xdt-csource, which
# xfce4-dev-tools 4.20 no longer ships. The tarball has them all.
TARBALL=https://archive.xfce.org/src/xfce/libxfce4ui/4.20/libxfce4ui-4.20.2.tar.bz2
SHA256=5d3d67b1244a10cee0e89b045766c05fe1035f7938f0410ac6a3d8222b5df907
DEPS="glib gtk3 libxfce4util xfconf libepoxy"
# hicolor-icon-theme at run time: libxfce4ui installs its icons (org.xfce.about
# and friends) under share/icons/hicolor, but GTK finds nothing there without
# hicolor's index.theme, and no DT_NEEDED shows that dependency.  On a target
# without it xfce_gicon_from_name() returned NULL for every themed icon.
RUNTIME_DEPS="hicolor-icon-theme"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-linker-opts --disable-x11 --enable-wayland )
