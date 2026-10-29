# PCRE 1 (the legacy API; pcre2 is a separate component). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=8.45
TARBALL=https://downloads.sourceforge.net/project/pcre/pcre/8.45/pcre-8.45.tar.bz2
SHA256=4dae6fdcd2bb0bb6c37b5f97c33c2be954da743985369cddac3546e3218bffb8
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-unicode-properties --enable-pcre16 --enable-pcre32 --enable-jit )
