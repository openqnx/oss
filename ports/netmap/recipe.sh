# The netmap applications (pkt-gen, bridge, lb, nmreplay, tlem) from the
# upstream netmap tree. QNX 8.0's io-sock has netmap built in (/dev/netmap,
# VALE switches, netmap pipes) and the SDP ships the user-space side --
# <net/netmap.h>, <net/netmap_user.h>, <libnetmap.h>, libnetmap.so.1 and
# vale-ctl -- but none of these applications. So only apps/ is built here,
# against the SDP's headers and libnetmap: the SDP's copies carry QNX's
# changes (io-sock instance prefix from $SOCK, rings mapped through a shared
# memory handle instead of mmap() of the device), which upstream's
# libnetmap does not have. vale-ctl is not built: the SDP has it.
# At run time the SDP's libnetmap.so.1 must be on the target (a stock
# mkqnximage image lacks it). With io-sock's default tunables a VALE port's
# memory allocator holds two netmap endpoints (dev.netmap.priv_if_num=2,
# priv_ring_num=4, priv_buf_num=4098): pkt-gen and nmreplay between two
# ports or over a pipe work; bridge and lb need more endpoints (raise those
# sysctls), and bridge between two VALE switches was seen not to be woken
# by poll() on QNX -- see the test notes.
VERSION=14.0-git-20261027; GIT=https://github.com/luigirizzo/netmap.git; COMMIT=37ec6d91d4fd2d0bd3de375fbd2119c56812de57
BUILDSYS=custom
NM_APPS="pkt-gen bridge lb nmreplay tlem"
# apps/include/qnx_compat.h (added by the patch) is force-included: the
# thread-affinity, bzero/bcopy and clock/mmap-flag differences, in one place.
nm_cc() { "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall \
            -include "$S/apps/include/qnx_compat.h" "$@"; }
do_build() {
  local a A="$S/apps"
  # -DNO_PCAP: pkt-gen's pcap replay (-f) would make libpcap a run-time
  # dependency for one option; nmreplay replays pcap files without it.
  # The libraries: libnetmap (SDP), libsocket (getifaddrs, ether_aton),
  # libm (tlem, pkt-gen statistics).
  for a in pkt-gen bridge nmreplay tlem; do
    nm_cc -DNO_PCAP -I"$A/include" "$A/$a/$a.c" -o "$B/$a" -lnetmap -lsocket -lm || return 1
  done
  nm_cc -I"$A/include" "$A/lb/lb.c" "$A/lb/pkt_hash.c" -o "$B/lb" -lnetmap -lsocket -lm
}
do_install() {
  local a s
  install -d "$STAGE$PREFIX/bin" "$STAGE$PREFIX/share/man/man8"
  for a in $NM_APPS; do
    install -m 755 "$B/$a" "$STAGE$PREFIX/bin/$a"
    s=8; install -m 644 "$S/apps/$a/$a.$s" "$STAGE$PREFIX/share/man/man8/$a.$s" || return 1
  done
}
