VERSION=4.20.0
# The release tarball, not the git tag: Xfce keeps its generated sources
# (*-alias.h, *-aliasdef.c, the .ui C sources) out of git and behind
# "if MAINTAINER_MODE", and regenerating them needs xdt-csource, which
# xfce4-dev-tools 4.20 no longer ships. The tarball has them all.
TARBALL=https://archive.xfce.org/src/xfce/garcon/4.20/garcon-4.20.0.tar.bz2
SHA256=7fb8517c12309ca4ddf8b42c34bc0c315e38ea077b5442bfcc4509415feada8f
DEPS="glib gtk3 libxfce4util libxfce4ui"   # garcon-gtk: PKG_CHECK_MODULES libxfce4ui-2
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-linker-opts  )
