VERSION=1.24.10
SRCROOT="$SRC_DIR/gstreamer"          # same monorepo as the core
SUBDIR=subprojects/gst-plugins-base
DEPS="gstreamer orc libogg libvorbis opus libtheora zlib"
BUILDSYS=meson
MESON_ARGS_EXTRA=(
  -Dtests=disabled -Dexamples=disabled -Dtools=enabled
  -Dintrospection=disabled -Ddoc=disabled -Dnls=enabled
  # what we have ported
  -Dogg=enabled -Dvorbis=enabled -Dopus=enabled -Dtheora=enabled
  # desktop/Linux-only or not ported yet
  -Dalsa=disabled -Dcdparanoia=disabled -Dlibvisual=disabled
  -Dpango=disabled -Dtremor=disabled
  -Dx11=disabled -Dxvideo=disabled -Dxshm=disabled
  -Dgl=disabled -Dgl-graphene=disabled
)
