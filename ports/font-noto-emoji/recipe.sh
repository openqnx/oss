# Noto Color Emoji. Font files only. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.051
TARBALL=https://github.com/googlefonts/noto-emoji/archive/refs/tags/v2.051.tar.gz
SHA256=04f3d1e5605edebebac00a7a0becb390a4a3ead015066905b27935b30c18e745
FETCH_NAME=noto-emoji-$VERSION.tar.gz
FONT_FILTER='*/fonts/NotoColorEmoji.ttf'
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_build() { :; }
do_install() { qnx_fonts_install font-noto-emoji; }
