# libheif: HEIF and AVIF image file format decoder and encoder (ISO/IEC
# 23008-12), with heif-dec, heif-enc, heif-info, heif-thumbnailer.
VERSION=1.20.2; GIT=https://github.com/strukturag/libheif.git; REF=v1.20.2
DEPS="libde265 x265 aom dav1d openh264 libjpeg-turbo openjpeg openjph libwebp libpng libtiff zlib"
BUILDSYS=cmake
# Codecs, all built into libheif (no plugin directory to install and search):
#   HEVC: libde265 decodes, x265 encodes; AV1: aom both ways, dav1d decodes;
#   H.264 (openh264) decodes; JPEG (libjpeg-turbo) and JPEG 2000 (openjpeg)
#   both ways; HT-J2K (openjph) encodes. libsharpyuv (from libwebp) for RGB->YUV.
# Left out: heif-view (an SDL viewer; on-screen use is not what this port is
# for), the gdk-pixbuf loader (it would need the loader cache updated on
# install), the experimental VVC codecs, kvazaar/svt/rav1e (not ported), tests.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DWITH_FUZZERS=OFF
                   -DENABLE_PLUGIN_LOADING=OFF
                   -DWITH_LIBDE265=ON -DWITH_X265=ON -DWITH_AOM_DECODER=ON -DWITH_AOM_ENCODER=ON
                   -DWITH_DAV1D=ON -DWITH_DAV1D_PLUGIN=OFF -DWITH_OpenH264_DECODER=ON
                   -DWITH_JPEG_DECODER=ON -DWITH_JPEG_ENCODER=ON
                   -DWITH_OpenJPEG_DECODER=ON -DWITH_OpenJPEG_ENCODER=ON
                   -DWITH_OpenJPEG_DECODER_PLUGIN=OFF -DWITH_OpenJPEG_ENCODER_PLUGIN=OFF
                   -DWITH_OPENJPH_ENCODER=ON -DWITH_OPENJPH_ENCODER_PLUGIN=OFF
                   -DWITH_SvtEnc=OFF -DWITH_RAV1E=OFF -DWITH_KVAZAAR=OFF -DWITH_FFMPEG_DECODER=OFF
                   -DWITH_EXAMPLES=ON -DWITH_EXAMPLE_HEIF_VIEW=OFF -DWITH_GDK_PIXBUF=OFF
                   -DCMAKE_DISABLE_FIND_PACKAGE_SDL2=ON -DCMAKE_DISABLE_FIND_PACKAGE_Doxygen=ON )
