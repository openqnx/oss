VERSION=4.20.0
# The release tarball, not the git tag: Xfce keeps its generated sources
# (*-alias.h, *-aliasdef.c, the .ui C sources) out of git and behind
# "if MAINTAINER_MODE", and regenerating them needs xdt-csource, which
# xfce4-dev-tools 4.20 no longer ships. The tarball has them all.
TARBALL=https://archive.xfce.org/src/xfce/libxfce4ui/4.20/libxfce4ui-4.20.0.tar.bz2
SHA256=75e8996984f20375aadecd5c16f5147c211ed0bd26d7861ab0257561eb76eaee
DEPS="glib gtk3 libxfce4util xfconf libepoxy"
# hicolor-icon-theme at run time: libxfce4ui installs its icons (org.xfce.about
# and friends) under share/icons/hicolor, but GTK finds nothing there without
# hicolor's index.theme, and no DT_NEEDED shows that dependency.  On a target
# without it xfce_gicon_from_name() returned NULL for every themed icon.
RUNTIME_DEPS="hicolor-icon-theme"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-linker-opts --disable-x11 --enable-wayland )
