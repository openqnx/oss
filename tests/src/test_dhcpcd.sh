#!/bin/sh
# dhcpcd without a network: its version and compiled-in features, its list
# of DHCP option variables (-V), and its DHCP decoder -- a DHCPACK message
# built byte by byte here is fed to `dhcpcd -4 --dumplease -` (stdin), which
# must print every option as the variable a hook script would get, with the
# known values. Nothing is started and the target's interfaces are not
# touched (no -T: test mode would still take a pid file in /var/run).
echo "== dhcpcd =="
fail=0
# The programs are in sbin/, which is not on every PATH.
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
PATH=$L/sbin:$L/bin:$PATH; export PATH
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-dhcpcd.XXXXXX) || { echo "FAIL dhcpcd (mktemp)"; exit 1; }
cleanup() { cd /; rm -rf "$T"; }
trap cleanup EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

V=$(dhcpcd --version 2>&1)
echo "  info $(echo "$V" | head -n 1)"
case "$V" in "dhcpcd 10.5.2"*) ok "dhcpcd --version: 10.5.2" ;; *) bad "dhcpcd --version: $V" ;; esac
echo "$V" | grep -q "Compiled in features:.*INET .*ARP.*INET6.*DHCPv6" \
  && ok "compiled in: INET ARP INET6 DHCPv6" || bad "features: $(echo "$V" | grep features)"

dhcpcd -V > vars.txt 2>&1
grep -q "^DHCPv4 options:" vars.txt && grep -q "^DHCPv6 options:" vars.txt \
  && ok "-V lists the DHCPv4 and DHCPv6 option tables" || bad "-V: $(head -c 200 vars.txt)"
# The embedded definitions (dhcpcd-definitions.conf compiled in): option 3
# is "routers", 6 "domain_name_servers", 121 "classless_static_routes".
grep -E -q "^0*3 +routers" vars.txt && grep -E -q "^0*6 +domain_name_servers" vars.txt \
  && grep -E -q "^121 +classless_static_routes" vars.txt \
  && ok "-V: options 3, 6, 121 are routers, domain_name_servers, classless_static_routes" \
  || bad "-V option names: $(grep -E 'routers|domain_name_servers|classless' vars.txt | head -3 | tr '\n' ' ')"

# A DHCPACK (BOOTP header, magic cookie, options), RFC 2131/2132/3442.
z() { i=0; while [ $i -lt $1 ]; do printf '\000'; i=$((i+1)); done; }
{
printf '\002\001\006\000'          # op=BOOTREPLY htype=ether hlen=6 hops=0
printf '\022\064\126\170'          # xid 0x12345678
printf '\000\000\000\000'          # secs, flags
printf '\000\000\000\000'          # ciaddr
printf '\300\000\002\067'          # yiaddr 192.0.2.55
printf '\300\000\002\001'          # siaddr 192.0.2.1
printf '\000\000\000\000'          # giaddr
printf '\002\000\000\021\042\063'; z 10   # chaddr 02:00:00:11:22:33
z 64; z 128                        # sname, file
printf '\143\202\123\143'          # magic cookie 99.130.83.99
printf '\065\001\005'              # 53 message type: DHCPACK
printf '\066\004\300\000\002\001'  # 54 server identifier 192.0.2.1
printf '\063\004\000\000\016\020'  # 51 lease time 3600
printf '\001\004\377\377\377\000'  # 1 subnet mask 255.255.255.0
printf '\003\004\300\000\002\001'  # 3 router 192.0.2.1
printf '\006\010\300\000\002\065\300\000\002\066'  # 6 DNS 192.0.2.53 192.0.2.54
printf '\017\014example.test'      # 15 domain name
printf '\014\007qnxhost'           # 12 host name
printf '\171\006\010\012\300\000\002\376'  # 121 10.0.0.0/8 via 192.0.2.254
printf '\052\004\300\000\002\173'  # 42 NTP server 192.0.2.123
printf '\377'                      # end
} > ack.bin
n=$(wc -c < ack.bin | tr -d ' ')
[ "$n" = 315 ] && ok "DHCPACK message built: 315 bytes" || bad "DHCPACK size $n (want 315)"

dhcpcd -4 --dumplease - < ack.bin > dump.txt 2> dump.err; rc=$?
[ $rc -eq 0 ] && ok "dhcpcd -4 --dumplease - exits 0" || bad "--dumplease rc=$rc: $(head -c 300 dump.err)"
for kv in ip_address=192.0.2.55 subnet_mask=255.255.255.0 subnet_cidr=24 \
          broadcast_address=192.0.2.255 network_number=192.0.2.0 routers=192.0.2.1 \
          "domain_name_servers=192.0.2.53 192.0.2.54" host_name=qnxhost \
          domain_name=example.test "classless_static_routes=10.0.0.0/8 192.0.2.254" \
          ntp_servers=192.0.2.123 dhcp_lease_time=3600 dhcp_message_type=5 \
          dhcp_server_identifier=192.0.2.1; do
  grep -q -x "$kv" dump.txt && ok "lease: $kv" || bad "lease: $kv (got: $(grep "^${kv%%=*}=" dump.txt))"
done

# Without -4/-6 dhcpcd cannot tell which decoder to use and must refuse.
dhcpcd --dumplease - < ack.bin > fam.txt 2>&1; rc=$?
[ $rc -ne 0 ] && grep -q "Family not specified" fam.txt \
  && ok "--dumplease without -4/-6 is refused" || bad "no family: rc=$rc $(head -c 200 fam.txt)"

# A configuration file is parsed: an unknown keyword is reported by name.
printf 'unknownkeyword foo\n' > bad.conf
dhcpcd -f bad.conf -4 --dumplease - < ack.bin > conf.txt 2>&1
grep -q "unknown option: unknownkeyword" conf.txt && grep -q -x "ip_address=192.0.2.55" conf.txt \
  && ok "-f config: unknown keyword reported, lease still decoded" || bad "-f config: $(head -c 200 conf.txt)"

cleanup; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS dhcpcd"; exit 0; }
echo "FAIL dhcpcd ($fail check(s) failed)"; exit 1
