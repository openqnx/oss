# scapy: builds, dissects, reads and writes network packets; here for its CAN
# layers (scapy.layers.can: CAN frames, candump and pcap files, DBC-like
# signals; scapy.contrib.isotp, automotive UDS/OBD/GMLAN layers) and for live
# capture and injection on io-sock interfaces through /dev/bpf. Pure Python,
# built from the sdist because of the QNX patch; capture filters are compiled
# by libpcap (loaded with ctypes).
VERSION=2.8.0
TARBALL=https://files.pythonhosted.org/packages/80/06/f7e09b3240196326fb343d6bd931cafedd26ae1dfdbd7cd0183cae3029d3/scapy-2.8.0.tar.gz
SHA256=0eacd1f940d115611c1b8a4696f315b4b80caf2951e546893a3d0cd7f6bfd6fd
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 libpcap"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_build() { qnx_py_wheel_pure; }
do_install() { qnx_py_install && qnx_py_scripts "$B"/wheel/*.whl; }
