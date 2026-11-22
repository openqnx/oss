# log4cpp: a C++ logging library after log4j -- categories, appenders
# (file, rolling file, syslog, ostream, remote), layouts and configuration
# files. (Not log4cxx, which is ported separately.)
VERSION=1.1.6
TARBALL=https://downloads.sourceforge.net/log4cpp/log4cpp-1.1.6.tar.gz
SHA256=a036bc6bd6044479e6c456de7edd042b060ea5c843e47beb75f59baea9b20e3a
# The tarball's top directory is log4cpp/.
SUBDIR=log4cpp
BUILDSYS=autotools
# configure's snprintf check is a run-test, so a cross build assumes there
# is none and compiles log4cpp's bundled portable_vsnprintf, which drops
# characters on QNX ("hello %d" came out "hell42"). QNX's vsnprintf is
# C99-conforming: say so.
CONF_ENV=( ac_cv_func_snprintf=yes )
# As qnx-ports builds it: shared only, no doxygen/dot documentation, no
# IDSA (intrusion detection) appender.
CONF_ARGS_EXTRA=( --disable-static --disable-doxygen --disable-dot --without-idsa )
