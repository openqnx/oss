# SWIG, the Simplified Wrapper and Interface Generator: generates bindings
# for C and C++ libraries (Python, Perl, Tcl, Lua, Java, ... -- every target
# language module is built into the one swig program). Version as in QNX's
# own package list.
VERSION=4.3.1
TARBALL=https://downloads.sourceforge.net/project/swig/swig/swig-4.3.1/swig-4.3.1.tar.gz
SHA256=44fc829f70f1e17d635a2b4d69acab38896699ecc24aa023e516e0eabbec61b8
# pcre2: %rename("%(regex:...)") and the other regex-based name matching.
DEPS="pcre2"
# Autotools, not the CMake build: the release tarball's configure uses its
# pre-generated parser (CMakeLists.txt insists on bison).
BUILDSYS=autotools
# --without-alllang: the language probes only choose which test suites run
# (on the build machine); the swig program has every language module.
# PCRE2: the staged library, named directly -- otherwise configure runs the
# build machine's pcre2-config. (Its headers are on the staging include path;
# PCRE2_CFLAGS only has to be non-empty.)
# --disable-ccache: no ccache-swig, SWIG's old private ccache copy (the ccache
# port is the real one).
CONF_ENV=( PCRE2_CFLAGS=-D_QNX_SOURCE PCRE2_LIBS=-lpcre2-8 )
CONF_ARGS_EXTRA=( --without-alllang --with-pcre --disable-ccache )
