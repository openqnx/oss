# Noto Color Emoji. Font files only. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.048
TARBALL=https://github.com/googlefonts/noto-emoji/archive/refs/tags/v2.048.tar.gz
SHA256=e6396642172e3d5031bef5f381cc047a007588e73b26209ba1c47b3d1f8faa60
FETCH_NAME=noto-emoji-2.048.tar.gz
FONT_FILTER='*/fonts/NotoColorEmoji.ttf'
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_build() { :; }
do_install() { qnx_fonts_install font-noto-emoji; }
