# Lynx: the text-mode web browser (HTTP/HTTPS with OpenSSL, FTP, local
# files), with -dump/-source for scripted page rendering. Screen library:
# ncursesw (UTF-8).
VERSION=2.9.2
TARBALL=https://invisible-mirror.net/archives/lynx/tarballs/lynx2.9.2.tar.bz2
SHA256=7374b89936d991669e101f4e97f2c9592036e1e8cdaa7bafc259a77ab6fb07ce
# configure also finds and uses brotli and bzip2 (compressed transfer
# encodings and .bz2 files), libidn2 (international domain names) and
# libiconv (charset conversion).
DEPS="ncurses openssl zlib brotli bzip2 libidn2 libiconv"
BUILDSYS=autotools
# lynx.cfg and lynx.lss go to the package tree's own etc (hazard 124).
# --with-build-cc: the character-set tables are generated at build time by
# makeuctb, which has to run on the build machine. --disable-full-paths:
# helper programs (gzip, telnet, ...) are looked up on PATH at run time, not
# wherever configure found them on the build machine.
CONF_ARGS_EXTRA=( --sysconfdir=/usr/local/etc --with-build-cc=gcc
                  --with-screen=ncursesw --with-ssl="$STAGE$PREFIX" --with-zlib
                  --enable-ipv6 --disable-nls --enable-default-colors
                  --disable-full-paths --enable-externs )
