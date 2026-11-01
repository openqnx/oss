# cups-filters: the CUPS filters for everything but macOS -- PDF as the
# print job format: pdftopdf (page handling, on qpdf), pdftoraster and
# pdftops (Poppler), texttopdf (fontconfig), imagetopdf/imagetoraster,
# rastertopdf, rastertopclx, the PWG/Apple raster and PCLm filters and
# driverless (PPDs for IPP Everywhere printers), with libcupsfilters and
# libfontembed; plus cups-browsed (legacy CUPS browsing only, see below).
VERSION=1.28.17
TARBALL=https://github.com/OpenPrinting/cups-filters/releases/download/1.28.17/cups-filters-1.28.17.tar.xz
SHA256=270a3752a960368aa99d431fb5d34f4039b2ac943c576d840612d1d8185c9bb9
BUILDSYS=autotools
DEPS="cups poppler qpdf fontconfig freetype lcms2 libjpeg-turbo libpng libtiff zlib glib"
RUNTIME_DEPS="cups"
# Off: Ghostscript and MuPDF (not ported -- the Poppler-based pdftops and
# pdftoraster take their place), Foomatic, Braille, LDAP, D-Bus colour
# management, Avahi (cups-browsed then browses only CUPS's own protocol),
# NLS. pdftops runs Poppler's pdftops program.
# cups-config: the staged one (it names the target paths: the filters go
# to CUPS's server directory, $PREFIX/lib/cups/filter).
CONF_ARGS_EXTRA=( --disable-static --enable-shared --disable-ghostscript --disable-mutool
                  --disable-foomatic --disable-braille --disable-ldap --disable-dbus
                  --disable-avahi --disable-nls --enable-poppler --enable-pclm --enable-driverless
                  --with-pdftops=pdftops --with-pdftops-path="$PREFIX/bin/pdftops"
                  --with-ippfind-path="$PREFIX/bin/ippfind"
                  --with-cups-config="$STAGE$PREFIX/bin/cups-config"
                  --with-fontdir="$PREFIX/share/fonts" --without-rcdir --with-test-font-path=/dev/null )
# cupsd starts filters with a clean environment and passes no LD_* variable
# on (see ports/cups): DT_RPATH $PREFIX/lib so the filters find libcups,
# libqpdf, libpoppler. -z stack-size: 8 MiB main stack like ports/cups
# (QNX's default main-thread stack is small).
LDFLAGS_APPEND="-Wl,-rpath,$PREFIX/lib -Wl,--disable-new-dtags -Wl,-z,stack-size=8388608 -Wl,-z,noexecstack"
