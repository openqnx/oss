VERSION=20240808-3.1
TARBALL=https://thrysoee.dk/editline/libedit-20240808-3.1.tar.gz
SHA256=5f0573349d77c4a48967191cdd6634dd7aa5f6398c6a57fe037cc02696d6099f
DEPS="ncurses"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --enable-widec )
# libedit's history search uses POSIX regex, which QNX keeps in its own
# libregex rather than in libc. Without this the shared library is linked with
# regcomp/regexec/regfree unresolved and every consumer fails to link.
CONF_ENV=( LIBS="-lregex" )
