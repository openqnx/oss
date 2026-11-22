VERSION=0.6.26; GIT=https://github.com/libexif/libexif.git; REF=v0.6.26
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-docs --disable-nls )
# Its own headers (libexif/*.h, included as <libexif/...>) ahead of the
# staging tree's: on an update the installed headers of the version before
# would win (exif_convert_utf16_to_utf8 changed its arguments in 0.6.26).
CONF_ENV=( CPPFLAGS="-I$SRC_DIR/$NAME -D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include" )
