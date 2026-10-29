# MATE Calculator (GTK3).
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.28.0
TARBALL=https://pub.mate-desktop.org/releases/1.28/mate-calc-1.28.0.tar.xz
SHA256=804b125d1e2864b1e74af816da9b2ab8b19472b9af974437ee7355ada5e628f5
DEPS="glib gtk3 libxml2 gmp mpfr mpc"
BUILDSYS=autotools
# itstool only builds the help pages (it would have to run on the build host).
CONF_ENV=( ITSTOOL=/bin/true )
CONF_ARGS_EXTRA=( --disable-nls )
