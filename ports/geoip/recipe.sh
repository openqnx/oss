# GeoIP: MaxMind's legacy C library (and geoiplookup) for the GeoIP v1
# country/city database format. Superseded upstream by libmaxminddb; kept
# for software that still reads .dat databases. No database is shipped.
VERSION=1.6.12
TARBALL=https://github.com/maxmind/geoip-api-c/releases/download/v1.6.12/GeoIP-1.6.12.tar.gz
SHA256=1dfb748003c5e4b7fd56ba8c4cd786633d5d6f409547584f6910398389636f80
BUILDSYS=autotools
# Databases are looked for in $PREFIX/share/GeoIP (datadir default).
CONF_ARGS_EXTRA=( --disable-static )
# AC_FUNC_MALLOC/AC_FUNC_REALLOC are run-tests; cross configure assumes the
# worst and substitutes rpl_malloc/rpl_realloc that nothing defines. QNX
# documents malloc(0) as returning a valid pointer (hazard 73).
CONF_ENV=( ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes )
