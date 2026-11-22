VERSION=4.20.1
# The release tarball, not the git tag: Xfce keeps its generated sources
# (*-alias.h, *-aliasdef.c, the .ui C sources) out of git and behind
# "if MAINTAINER_MODE", and regenerating them needs xdt-csource, which
# xfce4-dev-tools 4.20 no longer ships. The tarball has them all.
TARBALL=https://archive.xfce.org/src/xfce/libxfce4util/4.20/libxfce4util-4.20.1.tar.bz2
SHA256=84bfc4daab9e466193540c3665eee42b2cf4d24e3f38fc3e8d1e0a2bebe3b8f1
DEPS="glib"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-linker-opts  )
