# gd (libgd): a library for drawing images -- lines, polygons, text with
# FreeType fonts, fills, copies and scaling -- read and written as PNG, JPEG,
# GIF, WebP, TIFF, BMP, AVIF and HEIF. graphviz's gd renderer is built on it.
VERSION=2.3.3; GIT=https://github.com/libgd/libgd.git; REF=gd-2.3.3
DEPS="zlib libpng libjpeg-turbo libtiff libwebp freetype fontconfig libavif libheif"
BUILDSYS=cmake
# Every format whose library is here; text with FreeType and fontconfig font
# names. Not: XPM (needs libXpm, X11), libimagequant and raqm (not ported).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF -DBUILD_TEST=OFF
                   -DENABLE_PNG=ON -DENABLE_JPEG=ON -DENABLE_TIFF=ON -DENABLE_WEBP=ON
                   -DENABLE_FREETYPE=ON -DENABLE_FONTCONFIG=ON -DENABLE_AVIF=ON -DENABLE_HEIF=ON
                   -DENABLE_XPM=OFF -DENABLE_LIQ=OFF -DENABLE_RAQM=OFF -DENABLE_ICONV=OFF )
