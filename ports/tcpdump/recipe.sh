VERSION=4.99.7
TARBALL=https://www.tcpdump.org/release/tcpdump-4.99.7.tar.gz
SHA256=8be364e28d3b745ef1459b385cd2f4bc0e1ebad7a5d2ebdf70071d6c9b5b9a54
BUILDSYS=autotools
# On the libpcap port (static libpcap.a, found through its pcap-config).
DEPS="libpcap"
CONF_ENV=( PCAP_CONFIG="$STAGE$PREFIX/bin/pcap-config" )
# Without OpenSSL: the ESP decryption (-E) is the only thing it adds.
CONF_ARGS_EXTRA=( --without-crypto --without-smi --without-cap-ng --disable-local-libpcap )
