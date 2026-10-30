# lzip: the lzip compressor (LZMA, with integrity checking; the format GNU publishes some sources in).
VERSION=1.26
TARBALL=https://download.savannah.gnu.org/releases/lzip/lzip-1.26.tar.gz
SHA256=641cf30961525cbe3b340cc883436c8854e9f5032f459f444de4782b621e6572
DEPS=""
BUILDSYS=autotools
# Its own configure (not autoconf): the C++ compiler by name.
CONF_ARGS_EXTRA=( CXX="q++ -V${QCC_TARGET}_cxx" )
