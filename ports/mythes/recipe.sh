# MyThes: the thesaurus library of OpenOffice/LibreOffice (libmythes-1.2):
# looks a word up in a thesaurus data file (.dat) through its index (.idx)
# and returns its meanings and synonyms; th_gen_idx.pl writes the index of a
# .dat file. No thesaurus data is included (per-language, e.g.
# th_en_US_v2.dat). Version as in QNX's own package list (the latest release).
VERSION=1.2.4
TARBALL=https://downloads.sourceforge.net/hunspell/mythes-1.2.4.tar.gz
SHA256=1e81f395d8c851c3e4e75b568e20fa2fa549354e75ab397f9de4b0e0790a305f
BUILDSYS=autotools
# hunspell: the example program (not installed) stems words with it; the
# library itself does not use it.
DEPS="hunspell"
# th_gen_idx.pl names /usr/bin/perl; perl is the ported one in the prefix.
do_install() {
  make -C "$B" DESTDIR="$STAGE" install &&
  sed -i '1s|^#!/usr/bin/perl|#!/usr/local/bin/perl|' "$STAGE$PREFIX/bin/th_gen_idx.pl"
}
