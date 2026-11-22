# OpenSSH client and server. Version as in the QNX Everywhere 8.0.4r1 image.
# (The QNX SDP ships its own OpenSSH; this one installs under /usr/local.)
VERSION=10.6p2
TARBALL=https://cdn.openbsd.org/pub/OpenBSD/OpenSSH/portable/openssh-10.6p2.tar.gz
SHA256=9ae44872090cf2f06b3cf9ff147e7f16f309671340763ed2f99248398ae29d5b
# Also linked at run time (found by scripts/gen-dependency-index): libxcrypt
DEPS="openssl zlib libxcrypt"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --sysconfdir=/usr/local/etc/ssh --with-privsep-path=/var/empty --with-pid-dir=/var/run
                  --without-pam --without-selinux --disable-strip --with-ssl-dir="$STAGE$PREFIX" --with-zlib="$STAGE$PREFIX" )
# The QNX patches change configure.ac; regenerate configure from it.
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
