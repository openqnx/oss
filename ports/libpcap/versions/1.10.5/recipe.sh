VERSION=1.10.5
TARBALL=https://www.tcpdump.org/release/libpcap-1.10.5.tar.gz
SHA256=37ced90a19a302a7f32e458224a00c365c117905c2cd35ac544b6880a81488f0
BUILDSYS=autotools
# QNX has no SocketCAN/netlink; the BPF-based capture path is the right one.
CONF_ARGS_EXTRA=( --enable-shared --with-pcap=bpf --disable-dbus
                  --disable-rdma --without-libnl )
