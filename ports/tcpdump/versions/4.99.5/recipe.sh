VERSION=4.99.5
TARBALL=https://www.tcpdump.org/release/tcpdump-4.99.5.tar.gz
SHA256=8c75856e00addeeadf70dad67c9ff3dd368536b2b8563abf6854d7c764cd3adb
BUILDSYS=autotools
# On the libpcap port (static libpcap.a, found through its pcap-config).
DEPS="libpcap"
CONF_ENV=( PCAP_CONFIG="$STAGE$PREFIX/bin/pcap-config" )
# Without OpenSSL: the ESP decryption (-E) is the only thing it adds.
CONF_ARGS_EXTRA=( --without-crypto --without-smi --without-cap-ng --disable-local-libpcap )
