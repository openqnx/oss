# libasyncns: asynchronous name service queries (getaddrinfo, getnameinfo,
# res_query) run in a helper thread, for single-threaded event loops
# (PulseAudio's optional resolver, among others).
VERSION=0.8
TARBALL=https://0pointer.de/lennart/projects/libasyncns/libasyncns-0.8.tar.gz
SHA256=4f1a66e746cbe54ff3c2fbada5843df4fbbbe7481d80be003e8d11161935ab74
BUILDSYS=autotools
# The 2008 tarball's config.sub predates the *-nto-qnx8.0.0 triples:
# autoreconf replaces the generated build system with the host's current one.
do_patch() { ( cd "$SRCROOT" && autoreconf -fi ); }
# No lynx-made HTML documentation.
CONF_ARGS_EXTRA=( --disable-static --disable-lynx )
# AC_FUNC_MALLOC/AC_FUNC_REALLOC are run-tests; cross configure assumes the
# worst and substitutes rpl_malloc/rpl_realloc that nothing defines. QNX
# documents malloc(0) as returning a valid pointer (hazard 73).
CONF_ENV=( ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes )
