VERSION=4.20.0
# The release tarball, not the git tag: Xfce keeps its generated sources
# (*-alias.h, *-aliasdef.c, the .ui C sources) out of git and behind
# "if MAINTAINER_MODE", and regenerating them needs xdt-csource, which
# xfce4-dev-tools 4.20 no longer ships. The tarball has them all.
TARBALL=https://archive.xfce.org/src/xfce/exo/4.20/exo-4.20.0.tar.bz2
SHA256=4277f799245f1efde01cd917fd538ba6b12cf91c9f8a73fe2035fd5456ec078d
DEPS="glib gtk3 libxfce4util libxfce4ui"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-linker-opts  )
