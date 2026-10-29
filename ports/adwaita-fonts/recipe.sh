# Adwaita Sans and Mono, GNOME's fonts. Font files only. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=49.0
TARBALL=https://download.gnome.org/sources/adwaita-fonts/49/adwaita-fonts-49.0.tar.xz
SHA256=3157c620eb5b72b25ab156d194aa4eb223f9870d547fe83fdbdf06d3e7becb37
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_build() { :; }
do_install() { qnx_fonts_install adwaita-fonts; }
