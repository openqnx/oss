# flex, the fast lexical analyser generator (scanners from .l files), and
# libfl (its yywrap/main helpers). Version as in QNX's own package list.
VERSION=2.6.4
TARBALL=https://github.com/westes/flex/releases/download/v2.6.4/flex-2.6.4.tar.gz
SHA256=e87aae032bf07c26f85ac0ed3250998c37621d95f8bd748b31f15b33c45ee995
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
# --disable-nls: no gettext/libiconv (the C locale needs no catalogue).
CONF_ARGS_EXTRA=( --disable-nls --disable-rpath --without-libiconv-prefix --without-libintl-prefix )
