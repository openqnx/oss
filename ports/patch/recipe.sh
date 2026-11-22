# patch: GNU patch -- applies diff files to originals (on the target, for building there).
VERSION=2.8.1
TARBALL=https://ftp.gnu.org/gnu/patch/patch-2.8.1.tar.xz
SHA256=3240c7959c5347c8ed347e3c4201c0337bf4240d5cf09d68889e58eeaba30e17
DEPS=""
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-xattr )
