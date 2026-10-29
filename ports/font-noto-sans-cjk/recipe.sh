# Noto Sans CJK (Chinese, Japanese, Korean). Font files only. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.004
TARBALL=https://github.com/notofonts/noto-cjk/releases/download/Sans2.004/03_NotoSansCJK-OTC.zip
SHA256=528f4e1b25ff3badb0321b38d015d954c4c0de926c7830ef50e4a1948f6a3eed
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_build() { :; }
do_install() { qnx_fonts_install font-noto-sans-cjk; }
