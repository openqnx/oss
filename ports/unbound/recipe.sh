VERSION=1.24.1; GIT=https://github.com/NLnetLabs/unbound.git; REF=release-1.24.1
DEPS="openssl expat"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --with-ssl="$STAGE$PREFIX" --with-libexpat="$STAGE$PREFIX" --disable-static --enable-shared
                  --with-pidfile=/var/run/unbound.pid --with-conf-file=/usr/local/etc/unbound/unbound.conf )
# AC_PROG_LEX looks for a target libfl to link and, finding none in a cross
# build, gives up on flex; the generated lexer needs no library (it is built
# with %option noyywrap), so say so.
# configure probes the Sun compiler flag -xc99; qcc accepts it but reads it as
# "-x c99" (input language), and every later link test then fails -- which
# surfaces as "OpenSSL ... version 0.9.7 or higher is required".
CONF_ENV=( ac_cv_lib_lex="none needed" cv_prog_cc_flag_xc99=no )
