VERSION=1.24.10
SRCROOT="$SRC_DIR/gstreamer"
SUBDIR=subprojects/gst-libav
DEPS="gst-plugins-base ffmpeg"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Ddoc=disabled -Dtests=disabled )
