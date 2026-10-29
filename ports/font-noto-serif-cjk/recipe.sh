# Noto Serif CJK (Chinese, Japanese, Korean). Font files only. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.003
TARBALL=https://github.com/notofonts/noto-cjk/releases/download/Serif2.003/04_NotoSerifCJKOTC.zip
SHA256=c76ed8fd491ce98182ec6430238b90a1eabea8857c7db64e9037efa38e198a31
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_build() { :; }
do_install() { qnx_fonts_install font-noto-serif-cjk; }
