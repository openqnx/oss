# iperf 2 (the iperf2 project on SourceForge, not esnet's iperf3; installs
# bin/iperf). The QNX support is qnx-ports/iperf2 (branch qnx-2-2-1, based on
# upstream's 2-2-1 branch = this release): QNX 7.1/8.0 cases for the
# recvmmsg()/sendmmsg() batched UDP paths and the headers they need.
VERSION=2.2.1
TARBALL=https://downloads.sourceforge.net/project/iperf2/iperf-2.2.1.tar.gz
SHA256=754ab0a7e28033dbea81308ef424bc7df4d6e2fe31b60cc536b61b51fefbd8fb
BUILDSYS=autotools
