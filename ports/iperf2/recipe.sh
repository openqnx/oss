# iperf 2 (the iperf2 project on SourceForge, not esnet's iperf3; installs
# bin/iperf). No QNX patch since 2.2.2: upstream took qnx-ports/iperf2's
# recvmmsg()/sendmmsg() batched UDP paths, generalized under HAVE_RECVMMSG /
# HAVE_SENDMMSG (configure checks the functions), and the QNX uh_sum case.
VERSION=2.2.2
TARBALL=https://downloads.sourceforge.net/project/iperf2/iperf-2.2.2.tar.gz
SHA256=4008e1dc7cdeb68605747e11e954a5b25f4646ab6d71207d979acec5de7d385a
BUILDSYS=autotools
