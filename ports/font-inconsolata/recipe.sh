# Inconsolata, a monospace font for code and terminals (the variable-width
# family's static OpenType instances: regular to black, ultra-condensed to
# ultra-expanded). Font files only. Version as in QNX's own package list.
VERSION=3.000
TARBALL=https://github.com/googlefonts/Inconsolata/releases/download/v3.000/fonts_otf.zip
FETCH_NAME=Inconsolata-3.000-fonts_otf.zip
SHA256=a1d30ccd1ce9298fb8f172df3e73fb871e99ef0a7bdaba74a958736bd0de5bf1
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_build() { :; }
do_install() { qnx_fonts_install font-inconsolata; }
