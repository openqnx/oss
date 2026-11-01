# PTPd 2.x, the IEEE 1588 Precision Time Protocol daemon (the 2.3.1 release).
VERSION=2.3.1; GIT=https://github.com/ptpd/ptpd.git; REF=ptpd.2.3.1
BUILDSYS=autotools
# The git tree has no configure: qnxbuild runs autoreconf.
# UDP transport only (IPv4 multicast, hybrid or unicast): no libpcap
# Ethernet transport, no net-snmp MIB.
CONF_ARGS_EXTRA=( --disable-pcap --disable-snmp --enable-statistics )
# QNX's <net/ethernet.h> needs <sys/cdefs.h> (__packed) before it, which
# configure's bare header probe does not include: tell configure what is
# there (struct ether_addr with the BSD "octet" member). ptpd's own sources
# include it after <sys/socket.h>, which brings <sys/cdefs.h>.
# QNX's struct utmp has ut_type/OLD_TIME but no ut_tv: no utmp/wtmp records
# of clock steps (the only use ptpd has for utmp).
# malloc(0) returns a valid pointer on QNX (hazard 73).
CONF_ENV=( ac_cv_func_malloc_0_nonnull=yes ac_cv_header_net_ethernet_h=yes ac_cv_member_struct_ether_addr_octet=yes ac_cv_header_utmp_h=no )
