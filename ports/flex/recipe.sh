# flex, the fast lexical analyser generator (scanners from .l files), and
# libfl (its yywrap/main helpers). Version as in QNX's own package list.
VERSION=2.7.3
TARBALL=https://github.com/westes/flex/releases/download/v2.7.3/flex-2.7.3.tar.gz
SHA256=b18bbc3622b2b9ebecd0423f4e110adfad8db51d0c5ada3f26660c7cb7c735ec
BUILDSYS=autotools
# flex calls m4 at run time to expand its skeleton: the m4 port's
# /usr/local/bin/m4 (QNX itself ships none), not the build machine's path.
RUNTIME_DEPS="m4"
# regcomp/regexec are in QNX's libregex, not in libc: on LDFLAGS, not LIBS --
# the build also compiles a stage1flex for the build machine (with $(LIBS)),
# which has no libregex.
# malloc(0)/realloc(NULL, 0): a cross configure cannot run its test and
# guesses "no", which replaces them with rpl_malloc/rpl_realloc that flex
# never defines; QNX's return a valid pointer.
LDFLAGS_APPEND="-lregex"
CONF_ENV=( M4=/usr/local/bin/m4 ac_cv_path_M4=/usr/local/bin/m4
           ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes )
# 2.7 also runs m4 at build time: src/mkskel.sh expands the skeletons with
# $(m4), and the build-machine stage1flex that generates scan.c execs $M4
# (environment first, then its compiled-in path). configure sets both to the
# run-time path above, which is not on the build machine; on the make command
# line (so exported to stage1flex too) they become the build machine's m4
# from PATH. The installed flex keeps /usr/local/bin/m4 from config.h.
do_build() { make -C "$B" -j"$(nproc)" m4=m4 M4=m4; }
# --disable-nls: no gettext/libiconv (the C locale needs no catalogue).
CONF_ARGS_EXTRA=( --disable-nls --disable-rpath --without-libiconv-prefix --without-libintl-prefix )
