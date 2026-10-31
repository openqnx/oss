VERSION=4.20.7
# Not 4.20.0: its Wayland workspace-group object (ext-workspace) was broken
# upstream -- no instance-private data, no "handle" property, a finalize that
# unrefs itself -- and it spoke a 2023 draft of ext-workspace-v1 that current
# compositors (labwc) no longer offer. Any client on labwc crashed as soon as
# the compositor announced a workspace group. Fixed upstream in 4.20.x.
# The release tarball, not the git tag: Xfce keeps its generated sources
# (*-alias.h, *-aliasdef.c, the .ui C sources) out of git and behind
# "if MAINTAINER_MODE", and regenerating them needs xdt-csource, which
# xfce4-dev-tools 4.20 no longer ships. The tarball has them all.
TARBALL=https://archive.xfce.org/src/xfce/libxfce4windowing/4.20/libxfce4windowing-4.20.7.tar.bz2
SHA256=01320b279648ab5b13263f8d260bc5958be599e1eaac77c4503598dfcf96c8bb
DEPS="glib gtk3 wayland wayland-protocols"
BUILDSYS=autotools
# configure looks for wayland-scanner on PATH and finds nothing useful in a
# cross build; point it at the build-machine binary from hosttools/, which is
# the one that has to run.
# The host's wayland-scanner (hosttools/, or the build machine's own).
_wls="$WS/hosttools/bin/wayland-scanner"; [ -x "$_wls" ] || _wls=$(command -v wayland-scanner)
# 4.20.x ships xdt-gen-visibility in the tarball but looks for it only in
# $PWD (the build directory) and PATH: name it.
CONF_ENV=( WAYLAND_SCANNER="$_wls" XDT_GEN_VISIBILITY="${SRC_DIR:-$WS/src}/libxfce4windowing/xdt-gen-visibility" )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-linker-opts --disable-x11 --enable-wayland )
