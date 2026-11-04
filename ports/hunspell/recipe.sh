# Hunspell: the spell checker and morphological analyser (LibreOffice's,
# Firefox's, enchant's): libhunspell-1.7 and the hunspell program (-l, -a
# ispell pipe mode, -m, -s), plus affixcompress/analyze/chmorph/munch/
# unmunch for dictionary work. No dictionaries are included: they are
# per-language data (share/hunspell/<lang>.aff + .dic). Version as in QNX's
# own package list (also the latest release).
VERSION=1.7.2
TARBALL=https://github.com/hunspell/hunspell/releases/download/v1.7.2/hunspell-1.7.2.tar.gz
SHA256=11ddfa39afe28c28539fe65fc4f1592d410c1e9b6dd7d8a91ca25d85e9ec65b8
BUILDSYS=autotools
# iconv (GNU libiconv: QNX libc has none) converts dictionaries in other
# encodings than UTF-8.
DEPS="libiconv"
# --without-ui: the curses interactive front end is not built (the pipe and
# list modes, which programs and scripts use, are). No readline, no
# translations (libintl).
CONF_ARGS_EXTRA=( --without-ui --without-readline --disable-nls )
