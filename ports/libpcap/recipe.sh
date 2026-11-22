VERSION=1.11.0
TARBALL=https://www.tcpdump.org/release/libpcap-1.11.0.tar.gz
SHA256=596389bc8560ea027dff9db8aaf6c173d992366d9aef4baf5d7c6d180b4d49ad
BUILDSYS=autotools
# QNX has no SocketCAN/netlink; the BPF-based capture path is the right one.
CONF_ARGS_EXTRA=( --enable-shared --with-pcap=bpf --disable-dbus
                  --disable-rdma --without-libnl )
