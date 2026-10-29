VERSION=1.24.10
SRCROOT="$SRC_DIR/gstreamer"
SUBDIR=subprojects/gst-plugins-good
DEPS="gst-plugins-base flac libpng libjpeg-turbo libvpx lame zlib cairo libsoup gdk-pixbuf bzip2"
BUILDSYS=meson
MESON_ARGS_EXTRA=(
  -Dtests=disabled -Dexamples=disabled -Ddoc=disabled -Dnls=enabled
  # ported and enabled
  -Dflac=enabled -Dpng=enabled -Djpeg=enabled -Dvpx=enabled -Dlame=enabled
  # Linux-kernel or desktop-only subsystems QNX does not have
  -Dv4l2=disabled -Doss=disabled -Doss4=disabled -Dpulse=disabled
  -Djack=disabled -Dximagesrc=disabled -Dgtk3=disabled -Dqt5=disabled -Dqt6=disabled
  -Dwaveform=disabled -Ddirectsound=disabled -Dosxaudio=disabled -Dosxvideo=disabled
  # not ported (yet)
  -Dcairo=enabled -Dsoup=enabled -Dwavpack=disabled
  -Dspeex=disabled -Dtaglib=disabled -Dtwolame=disabled
  -Dmpg123=disabled -Dshout2=disabled -Ddv=disabled -Ddv1394=disabled
  -Daalib=disabled -Dlibcaca=disabled -Dgdk-pixbuf=enabled -Dbz2=enabled
  -Dadaptivedemux2=disabled
)
