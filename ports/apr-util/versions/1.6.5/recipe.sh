VERSION=1.6.5
TARBALL=https://archive.apache.org/dist/apr/apr-util-1.6.5.tar.bz2
SHA256=96de1dd6f6a0476d2d2e7964926d8c1ddc3bb0e210e1b1812d3ba5a454a392e2
BUILDSYS=autotools
DEPS="apr expat openssl sqlite libiconv libxcrypt"
# apr-1-config run from staging answers with staging paths (its
# "crosscompile" mode); the build rules and libtool come from apr.
# XML via the ported expat; apr_crypto with OpenSSL and the SQLite 3 DBD
# driver, both built as modules in lib/apr-util-1/ that apr-util dlopen()s
# from $PREFIX/lib/apr-util-1. No LDAP, PostgreSQL, MySQL, ODBC, Berkeley DB
# or gdbm (the built-in sdbm is the DBM). QNX's libc has neither iconv nor
# crypt(): apr_xlate uses GNU libiconv, apr_password_validate libxcrypt.
CONF_ARGS_EXTRA=( --with-apr="$STAGE$PREFIX/bin/apr-1-config" --with-expat="$STAGE$PREFIX"
  --with-crypto --with-openssl="$STAGE$PREFIX" --with-sqlite3="$STAGE$PREFIX"
  --without-ldap --without-pgsql --without-mysql --without-sqlite2 --without-odbc
  --without-berkeley-db --without-gdbm --without-ndbm --without-nss --without-commoncrypto )
# configure copies apr's apr_rules.mk, which names apr's helpers (libtool,
# make_exports.awk) where apr installs them on the target; the build takes
# them from staging.
do_build() {
  sed -i -E "s#^(apr_builddir|apr_builders|top_builddir)=$PREFIX/share/apr-1/build#\1=$STAGE$PREFIX/share/apr-1/build#" "$B/build/rules.mk" &&
  make -C "$B" -j"$(nproc)"
}
# As for apr-1-config: the source and build trees are only for uninstalled use.
do_install() {
  make -C "$B" DESTDIR="$STAGE" install &&
  sed -i -e 's|^APU_SOURCE_DIR=.*|APU_SOURCE_DIR=""|' -e 's|^APU_BUILD_DIR=.*|APU_BUILD_DIR=""|' "$STAGE$PREFIX/bin/apu-1-config"
}
