# openresolv: resolvconf, the framework that merges the DNS information of
# several sources (DHCP clients, VPNs, static configuration) into
# resolv.conf, and optionally into local resolver configuration (unbound,
# dnsmasq, named, pdnsd). Shell scripts, the same for every architecture.
VERSION=3.17.4
TARBALL=https://github.com/NetworkConfiguration/openresolv/releases/download/v3.17.4/openresolv-3.17.4.tar.xz
SHA256=901d83a15520b80e117aeca8cb6f5b70ceb205ba050a638d45cebcf304df729b
BUILDSYS=custom
# Its own configure (not autoconf). --os=qnx: it would otherwise take the
# build machine's uname (Linux) and expect sha256sum/gpg2 there.
# resolvconf.conf lives in the package tree's own etc (hazard 124); the
# file written stays the system's /etc/resolv.conf, which QNX's resolver
# reads (resolv_conf= in resolvconf.conf changes it). The interface state
# goes under /var/run/resolvconf.
do_build() { rm -rf "$B" && cp -a "$S" "$B" && cd "$B" &&
  ./configure --os=qnx --prefix="$PREFIX" --sysconfdir="$PREFIX/etc" --sbindir="$PREFIX/sbin" \
    --libexecdir="$PREFIX/libexec/resolvconf" --rundir=/var/run --mandir="$PREFIX/share/man" && make; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
