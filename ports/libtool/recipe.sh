VERSION=2.5.4; TARBALL=https://ftp.gnu.org/gnu/libtool/libtool-2.5.4.tar.gz
SHA256=da8ebb2ce4dcf46b90098daf962cffa68f4b4f62ea60f798d0ef12929ede6adf
BUILDSYS=autotools
# Only libltdl is useful on the target; the m4 macros and the wrapper script are
# build-machine tools and already present on this host.
SUBDIR=libltdl
CONF_ARGS_EXTRA=( --enable-shared --disable-static --enable-ltdl-install )
