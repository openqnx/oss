VERSION=1.24.10
SRCROOT="$SRC_DIR/gstreamer"
SUBDIR=subprojects/gst-plugins-bad
DEPS="gst-plugins-base x265 openh264 aom svt-av1 fdk-aac libwebp openssl pango cairo"
BUILDSYS=meson
# This plugin set probes for well over a hundred optional dependencies, most of
# them desktop-Linux only. Turn the auto-detection off and name what we ported.
MESON_ARGS_EXTRA=(
  -Dtests=disabled -Dexamples=disabled -Ddoc=disabled -Dnls=enabled
  -Dauto_features=disabled
  -Dgpl=enabled
  # codecs we built from source in this workspace
  -Dx265=enabled -Dopenh264=enabled -Daom=enabled
  -Dsvtav1=enabled -Dfdkaac=enabled -Dwebp=enabled
  # pure-C element sets with no external dependency
  -Dvideoparsers=enabled -Dmpegtsdemux=enabled -Dmpegtsmux=enabled
  -Dmpegpsmux=enabled -Dclosedcaption=enabled -Dcodecalpha=enabled
  -Dcodectimestamper=enabled -Dinterlace=enabled -Divfparse=enabled -Divtc=enabled
  -Djpegformat=enabled -Dmxf=enabled -Dpcapparse=enabled -Dsdp=enabled
  -Dsegmentclip=enabled -Dsiren=enabled -Dspeed=enabled -Dsubenc=enabled
  -Dtimecode=enabled -Dvideofilters=enabled -Dvideoframe_audiolevel=enabled
  -Dy4m=enabled -Daudiobuffersplit=enabled -Daudiolatency=enabled
  -Daudiomixmatrix=enabled -Dautoconvert=enabled -Dbayer=enabled
  -Dcamerabin2=enabled -Ddebugutils=enabled -Dfieldanalysis=enabled
  -Dfreeverb=enabled -Dgaudieffects=enabled -Dgeometrictransform=enabled
  -Dinter=enabled -Dproxy=enabled -Drawparse=enabled -Dremovesilence=enabled
  -Dsmooth=enabled -Dspeed=enabled -Dvideosignal=enabled -Dwebrtcdsp=disabled
)
