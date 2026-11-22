VERSION=1.33.7
TARBALL=https://www.mpg123.de/download/mpg123-1.33.7.tar.bz2
SHA256=31d0e35a4ca567ec9b5ebda6c3062bb4435d6d3eacd6ef0d95cadd7854dc03ee
BUILDSYS=autotools
# The decoder (libmpg123) and resampler (libsyn123) libraries only. QNX audio
# goes through io-snd/libasound and mpg123 has no output module for it, so
# libout123 and the player are left out: applications decode with libmpg123 and
# play through their own audio path. --disable-components turns everything off;
# each --enable-lib* then turns one library back on.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-cpu=generic_fpu
                  --disable-components --enable-libmpg123 --enable-libsyn123 )
