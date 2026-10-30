# patch: GNU patch -- applies diff files to originals (on the target, for building there).
VERSION=2.8
TARBALL=https://ftp.gnu.org/gnu/patch/patch-2.8.tar.xz
SHA256=f87cee69eec2b4fcbf60a396b030ad6aa3415f192aa5f7ee84cad5e11f7f5ae3
DEPS=""
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-xattr )
