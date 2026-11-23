VERSION=4.0
TARBALL=https://downloads.sourceforge.net/lame/lame-4.0.tar.gz
SHA256=3df5124d5ad3a98312ffd7ba6a9b36230e4f8a3e66d3ce0f425e336c32d216eb
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-frontend
                  --disable-gtktest --enable-nasm )
