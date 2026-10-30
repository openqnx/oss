# poppler: PDF rendering library (xpdf-based) with its command-line tools (pdftotext,
# pdfinfo, pdftoppm, pdftocairo, ...); C++, GLib and cairo front ends. No NSS
# (signature verification: NSS is not ported), no Qt front ends, no GPGME.
VERSION=25.12.0; GIT=https://gitlab.freedesktop.org/poppler/poppler.git; REF=poppler-25.12.0
DEPS="freetype fontconfig cairo glib libjpeg-turbo libpng libtiff openjpeg lcms2 zlib"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DENABLE_NSS3=OFF -DENABLE_GPGME=OFF -DENABLE_QT5=OFF -DENABLE_QT6=OFF -DENABLE_LIBCURL=OFF -DBUILD_GTK_TESTS=OFF -DBUILD_QT5_TESTS=OFF -DBUILD_QT6_TESTS=OFF -DBUILD_CPP_TESTS=OFF -DBUILD_MANUAL_TESTS=OFF -DENABLE_GLIB=ON -DENABLE_UTILS=ON -DENABLE_CPP=ON -DENABLE_UNSTABLE_API_ABI_HEADERS=OFF -DENABLE_GOBJECT_INTROSPECTION=OFF -DENABLE_LCMS=ON -DENABLE_DCTDECODER=libjpeg -DENABLE_LIBOPENJPEG=openjpeg2 -DENABLE_BOOST=OFF
                   -DPython3_EXECUTABLE=/usr/bin/python3 -DGLIB2_MKENUMS_PYTHON=/usr/bin/python3 )
# glib-mkenums runs on the build machine: its Python, not the QNX one CMake
# finds in the sysroot.
