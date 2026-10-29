# Liberation fonts (metric-compatible with Arial, Times New Roman, Courier New). Font files only. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.1.5
TARBALL=https://github.com/liberationfonts/liberation-fonts/files/7261482/liberation-fonts-ttf-2.1.5.tar.gz
SHA256=7191c669bf38899f73a2094ed00f7b800553364f90e2637010a69c0e268f25d0
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_build() { :; }
do_install() { qnx_fonts_install font-liberation; }
