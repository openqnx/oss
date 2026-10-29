VERSION=3.100
TARBALL=https://downloads.sourceforge.net/lame/lame-3.100.tar.gz
SHA256=ddfe36cab873794038ae2c1210557ad34857a4b6bdc515785d1da9e175b1da1e
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-frontend
                  --disable-gtktest --enable-nasm )
