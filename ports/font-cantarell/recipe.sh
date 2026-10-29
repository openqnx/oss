# Cantarell, the GNOME UI font (prebuilt OTFs from the release). Font files only. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=0.303.1
TARBALL=https://download.gnome.org/sources/cantarell-fonts/0.303/cantarell-fonts-0.303.1.tar.xz
SHA256=f9463a0659c63e57e381fdd753cf1929225395c5b49135989424761830530411
FONT_FILTER='*/prebuilt/*'
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_build() { :; }
do_install() { qnx_fonts_install font-cantarell; }
