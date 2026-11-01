VERSION=3.8.2
TARBALL=https://ftp.gnu.org/gnu/bison/bison-3.8.2.tar.xz
SHA256=9bba0214ccf7f1079c5d59210045227bcf619519840ebfa80cd3849cff5a5bf2
BUILDSYS=autotools
# bison runs m4 to expand its skeletons, at run time: the m4 port's
# /usr/local/bin/m4 (QNX itself ships none).
RUNTIME_DEPS="m4"
# M4: the program bison runs on the target, not a build-machine probe (a
# cross configure would otherwise record the build machine's m4 path).
# Neither libiconv nor gettext's libtextstyle (coloured diagnostics): the
# C locale needs no conversion, and both would otherwise be picked up from
# staging -- with the staging directory as an rpath (--disable-rpath).
CONF_ENV=( M4=/usr/local/bin/m4 ac_cv_path_M4=/usr/local/bin/m4 am_cv_func_iconv=no ac_cv_libtextstyle=no )
CONF_ARGS_EXTRA=( --disable-nls --disable-yacc --disable-rpath --without-libtextstyle-prefix --without-libiconv-prefix )
