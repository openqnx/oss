VERSION=1.1.1
TARBALL=https://downloads.xiph.org/releases/theora/libtheora-1.1.1.tar.bz2
SHA256=b6ae1ee2fa3d42ac489287d3ec34c5885730b1296f0801ae577a35193d3affbc
BUILDSYS=autotools
DEPS="libogg libvorbis"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-examples
                  --disable-spec --disable-doc
                  --with-ogg="$STAGE$PREFIX" --with-vorbis="$STAGE$PREFIX" )
