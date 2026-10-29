VERSION=4.16.7
# The release tarball, not the git tag. gtk/meson.build says it outright --
# "For git checkouts, but not for tarballs..." -- and from git it compiles the
# CSS themes with sassc, a *build-machine* program it falls back to building as
# a subproject and then running, which a cross build cannot do ("An exe_wrapper
# is needed but was not found"). The tarball ships the compiled themes.
TARBALL=https://download.gnome.org/sources/gtk/4.16/gtk-4.16.7.tar.xz
SHA256=5303c7624e15a48880591b9ddd43389afba3de4fb92a219719519bb1642ce3dc
SRCROOT="$SRC_DIR/gtk4"
DEPS="glib graphene cairo pango gdk-pixbuf harfbuzz fribidi libepoxy
      wayland wayland-protocols libxkbcommon libpng libjpeg-turbo libtiff"
BUILDSYS=meson
MESON_ARGS_EXTRA=(
  -Dwayland-backend=true -Dx11-backend=false -Dbroadway-backend=true
  -Dwin32-backend=false -Dmacos-backend=false
  -Df16c=disabled
  -Dvulkan=disabled -Dmedia-gstreamer=disabled -Dcloudproviders=disabled
  -Dsysprof=disabled -Dtracker=disabled -Dcolord=disabled
  -Dintrospection=disabled -Ddocumentation=false -Dscreenshots=false
  -Dman-pages=false -Dbuild-demos=false -Dbuild-testsuite=false
  -Dbuild-examples=false -Dbuild-tests=false -Dprint-cups=disabled
)
