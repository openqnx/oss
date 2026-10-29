# Inter. Font files only. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=4.1
TARBALL=https://github.com/rsms/inter/releases/download/v4.1/Inter-4.1.zip
SHA256=9883fdd4a49d4fb66bd8177ba6625ef9a64aa45899767dde3d36aa425756b11e
FONT_FILTER='*/extras/otf/*'
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_build() { :; }
do_install() { qnx_fonts_install font-inter; }
