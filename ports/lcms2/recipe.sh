VERSION=2.16; GIT=https://github.com/mm2/Little-CMS.git; REF=lcms2.16
DEPS="zlib"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dutils=false -Djpeg=disabled -Dtiff=disabled )
