VERSION=1.2.0
TARBALL=https://downloads.xiph.org/releases/theora/libtheora-1.2.0.tar.xz
SHA256=ebdf77a8f5c0a8f7a9e42323844fa09502b34eb1d1fece7b5f54da41fe2122ec
BUILDSYS=autotools
DEPS="libogg libvorbis"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-examples
                  --disable-spec --disable-doc
                  --with-ogg="$STAGE$PREFIX" --with-vorbis="$STAGE$PREFIX" )
