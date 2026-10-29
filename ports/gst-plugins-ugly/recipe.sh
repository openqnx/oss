VERSION=1.24.10
SRCROOT="$SRC_DIR/gstreamer"
SUBDIR=subprojects/gst-plugins-ugly
DEPS="gst-plugins-base x264"
BUILDSYS=meson
MESON_ARGS_EXTRA=(
  -Dtests=disabled -Ddoc=disabled -Dnls=enabled
  -Dgpl=enabled                 # x264 is GPL; the plugin set refuses to build it otherwise
  -Dx264=enabled
  # not ported: the DVD/legacy-codec corner of this plugin set
  -Da52dec=disabled -Dcdio=disabled
  -Ddvdread=disabled -Dmpeg2dec=disabled -Dsidplay=disabled
)
