VERSION=2.3.0
TARBALL=https://github.com/Juniper/libxo/releases/download/2.3.0/libxo-2.3.0.tar.gz
SHA256=f688acfbad07ba14826871437b0431bc0425d5732457013af5aff07236810f65
BUILDSYS=autotools
# No gettext: the text output stays untranslated (QNX has only the C locale).
# configure demands byacc (for the XPath filters) even when filters are not
# enabled, and they are off by default: name one so the check passes; no
# grammar is built.
# AC_FUNC_MALLOC/REALLOC are run-time probes; QNX documents that malloc(0)
# returns a valid pointer (hazard 73), and realloc(NULL, 0) is malloc(0).
# libxo_format would link OpenSSL's libcrypto when staging has it (an
# MD5_Init probe); it is not needed for anything libxo does here.
CONF_ENV=( YACC=byacc ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes
           ac_cv_lib_crypto_MD5_Init=no )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gettext )
