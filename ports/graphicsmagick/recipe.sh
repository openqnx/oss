# graphicsmagick: image processing (the gm command and the Magick++ / C libraries;
# PNG, JPEG, TIFF, WebP, JPEG XL, FreeType text).
VERSION=1.3.46
TARBALL=https://downloads.sourceforge.net/project/graphicsmagick/graphicsmagick/1.3.46/GraphicsMagick-1.3.46.tar.xz
SHA256=c7c706a505e9c6c3764156bb94a0c9644d79131785df15a89c9f8721d1abd061
DEPS="libpng libjpeg-turbo libtiff libwebp libjxl freetype libxml2 zlib bzip2 xz zstd lcms2"
BUILDSYS=autotools
# QNX has a spawnvp() (process.h), which configure finds; GraphicsMagick's
# spawnvp branch is the Windows one (_P_WAIT). The POSIX fork/exec one is
# right for QNX.
CONF_ENV=( ac_cv_func_spawnvp=no )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-modules=no --without-x --without-perl --without-trio --without-wmf --without-gs --disable-openmp --with-quantum-depth=16 )
