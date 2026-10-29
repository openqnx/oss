# Hack, a monospace font for code. Font files only. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=3.003
TARBALL=https://github.com/source-foundry/Hack/releases/download/v3.003/Hack-v3.003-ttf.tar.xz
SHA256=d9ed5d0a07525c7e7bd587b4364e4bc41021dd668658d09864453d9bb374a78d
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_build() { :; }
do_install() { qnx_fonts_install font-hack; }
