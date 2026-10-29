VERSION=1.24.10; GIT=https://gitlab.freedesktop.org/gstreamer/gstreamer.git; REF=1.24.10
DEPS="glib libxml2"
SUBDIR=subprojects/gstreamer
BUILDSYS=meson
MESON_ARGS_EXTRA=(
  -Dtests=disabled -Dexamples=disabled -Dbenchmarks=disabled
  -Dtools=enabled -Dgst_debug=true
  -Dintrospection=disabled -Ddoc=disabled
  -Dptp-helper-permissions=none     # setuid helper install needs root
  -Dlibunwind=disabled -Dlibdw=disabled -Ddbghelp=disabled
  -Dbash-completion=disabled
)
