# OpenSSH client and server. Version as in the QNX Everywhere 8.0.4r1 image.
# (The QNX SDP ships its own OpenSSH; this one installs under /usr/local.)
VERSION=10.2p1
TARBALL=https://cdn.openbsd.org/pub/OpenBSD/OpenSSH/portable/openssh-10.2p1.tar.gz
SHA256=ccc42c0419937959263fa1dbd16dafc18c56b984c03562d2937ce56a60f798b2
# Also linked at run time (found by scripts/gen-dependency-index): libxcrypt
DEPS="openssl zlib libxcrypt"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --sysconfdir=/usr/local/etc/ssh --with-privsep-path=/var/empty --with-pid-dir=/var/run
                  --without-pam --without-selinux --disable-strip --with-ssl-dir="$STAGE$PREFIX" --with-zlib="$STAGE$PREFIX" )
# The QNX patches change configure.ac; regenerate configure from it.
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
