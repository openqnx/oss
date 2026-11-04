# GNU Texinfo: makeinfo/texi2any (Texinfo to Info, HTML, plain text,
# DocBook, LaTeX), the stand-alone info reader, install-info, texindex and
# the texi2dvi/texi2pdf drivers (which need TeX). Version as in QNX's own
# package list.
VERSION=7.2
TARBALL=https://ftp.gnu.org/gnu/texinfo/texinfo-7.2.tar.xz
SHA256=0329d7788fbef113fa82cb80889ca197a344ce0df7646fe000974c5d714363a6
# ncurses: the info reader's terminal handling (tgetent).
DEPS="ncurses"
# texi2any, makeinfo and pod2texi are Perl programs (with Text::Unidecode,
# libintl-perl and Unicode::EastAsianWidth bundled in share/texi2any/lib);
# texindex is sh + awk.
RUNTIME_DEPS="perl"
BUILDSYS=autotools
# --disable-perl-xs: texi2any's optional XS modules (the faster C parser
# and converters) are built against the perl that configure runs -- the
# build machine's (its CORE headers, gcc, x86-64 Linux objects in a QNX
# package). Without them texi2any uses its pure-Perl implementation, which
# gives the same output.
# --disable-nls, am_cv_func_iconv=no: no gettext/libiconv in the C programs
# (the build machine's perl runs only to configure; PERL is its path).
# ac_cv_path_TI_AWK=awk: texindex runs the awk configure found, by its
# build-machine path (/usr/bin/awk); a QNX target's awk is /system/bin/awk
# (toybox), so texindex takes "awk" from PATH.
CONF_ENV=( PERL=/usr/bin/perl am_cv_func_iconv=no ac_cv_path_TI_AWK=awk )
CONF_ARGS_EXTRA=( --disable-perl-xs --disable-nls --disable-rpath
                  --without-libiconv-prefix --without-libintl-prefix )
do_install() {
  make -C "$B" DESTDIR="$STAGE" install || return 1
  local f
  # The Perl programs start with the build machine's #! /usr/bin/perl
  # (hazard 105); a QNX target's perl is the perl port's.
  for f in texi2any pod2texi; do
    sed -i "1s|^#! */usr/bin/perl|#!$PREFIX/bin/perl|" "$STAGE$PREFIX/bin/$f" || return 1
  done
}
