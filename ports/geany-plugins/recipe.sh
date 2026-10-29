# The Geany plugin collection (the plugins without extra library dependencies).
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.1
TARBALL=https://plugins.geany.org/geany-plugins/geany-plugins-2.1.tar.bz2
SHA256=813f24570ca7590a9ae6e8e9ca4e5bd6c9f09d61fe36ad6feca89b7c2feaae70
DEPS="geany"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-all-plugins --enable-addons --enable-autoclose --enable-automark --enable-codenav --enable-commander --enable-defineformat --enable-filebrowser --enable-keyrecord --enable-lineoperations --enable-overview --enable-pairtaghighlighter --enable-projectorganizer --enable-sendmail --enable-shiftcolumn --enable-treebrowser --enable-xmlsnippets )
# The QNX patch touches the autotools inputs; regenerate the build files.
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
