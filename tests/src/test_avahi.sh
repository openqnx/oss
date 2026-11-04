#!/bin/sh
# Avahi end to end on loopback: a private D-Bus system bus (dbus-daemon with
# its own config, avahi's bus policy included) and avahi-daemon on it,
# restricted to lo0 (host name qnxtest-<pid>, IPv4). Then, through the
# client library over D-Bus:
#  - avahi-browse-domains / the daemon's version string;
#  - avahi-publish-service announces a service with a TXT record;
#    avahi-browse -r resolves it: name, type, port 4242, the TXT record and
#    127.0.0.1;
#  - avahi-resolve -n resolves the daemon's own host name to 127.0.0.1 over
#    mDNS, and avahi-resolve -a maps 127.0.0.1 back;
#  - avahi-publish-address publishes an extra A record that resolves.
# Both daemons and every publisher are stopped on every exit path.
echo "== avahi =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-avahi.XXXXXX) || { echo "FAIL avahi (mktemp)"; exit 1; }
PIDS=
stop_all() {  # the last started first: publishers, then avahi-daemon, then the bus
  R=; for p in $PIDS; do R="$p $R"; done
  for p in $R; do
    kill $p 2>/dev/null
    i=0; while kill -0 $p 2>/dev/null && [ $i -lt 5 ]; do sleep 1; i=$((i+1)); done
    kill -9 $p 2>/dev/null; wait $p 2>/dev/null
  done
  PIDS=
}
cleanup() { stop_all; cd /; rm -rf "$T"; }
trap cleanup EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

# avahi-daemon's PID file: one daemon per system. Do not touch a running one.
RUNDIR=/usr/local/var/run/avahi-daemon
if [ -f $RUNDIR/pid ] && kill -0 "$(cat $RUNDIR/pid)" 2>/dev/null; then
  echo "  info an avahi-daemon is already running (pid $(cat $RUNDIR/pid)): not starting another"
  echo "SKIP avahi"; exit 0
fi

POLICY=
for d in /usr/local/share/dbus-1/system.d /data/oss/usr/local/share/dbus-1/system.d; do
  [ -f $d/avahi-dbus.conf ] && { POLICY=$d/avahi-dbus.conf; break; }
done
[ -n "$POLICY" ] && ok "avahi D-Bus policy installed" || bad "avahi-dbus.conf not found"
cat > bus.conf <<EOF
<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <type>system</type>
  <listen>unix:path=$T/bus</listen>
  <auth>EXTERNAL</auth>
  <policy context="default">
    <allow user="*"/>
    <allow send_destination="*" eavesdrop="true"/>
    <allow eavesdrop="true"/>
    <allow own="*"/>
  </policy>
  <include ignore_missing="yes">$POLICY</include>
</busconfig>
EOF
dbus-daemon --config-file=$T/bus.conf --nofork --nopidfile > dbus.log 2>&1 &
PIDS="$PIDS $!"
i=0; while [ ! -S $T/bus ] && [ $i -lt 50 ]; do sleep 0.2; i=$((i+1)); done
[ -S $T/bus ] && ok "private D-Bus system bus listening" || { bad "dbus-daemon: $(head -5 dbus.log)"; }
DBUS_SYSTEM_BUS_ADDRESS=unix:path=$T/bus; export DBUS_SYSTEM_BUS_ADDRESS

HN=qnxtest-$$
cat > avahi.conf <<EOF
[server]
host-name=$HN
domain-name=local
use-ipv4=yes
use-ipv6=no
allow-interfaces=lo0
enable-dbus=yes
ratelimit-interval-usec=1000000
ratelimit-burst=1000
[wide-area]
enable-wide-area=no
[publish]
publish-hinfo=no
publish-workstation=no
publish-addresses=yes
[reflector]
[rlimits]
EOF
avahi-daemon --no-drop-root --no-rlimits --no-proc-title -f $T/avahi.conf > daemon.log 2>&1 &
APID=$!; PIDS="$PIDS $APID"
i=0; while ! grep -q 'Server startup complete' daemon.log && [ $i -lt 60 ]; do kill -0 $APID 2>/dev/null || break; sleep 0.5; i=$((i+1)); done
if grep -q 'Server startup complete' daemon.log; then
  ok "avahi-daemon started: $(grep -m1 'Server startup complete' daemon.log | sed 's/.*Host name is //')"
else
  bad "avahi-daemon did not start: $(tail -5 daemon.log | tr '\n' ' ')"
fi
grep -q 'Joining mDNS multicast group on interface lo0.IPv4 with address 127.0.0.1' daemon.log \
  && ok "joined the mDNS group on lo0 (127.0.0.1)" || bad "no mDNS on lo0: $(grep -i -m3 'interface\|multicast' daemon.log | tr '\n' ' ')"

v=$(avahi-browse-domains --version 2>&1)
echo "  info $v"
case "$v" in *0.9-rc3*) ok "client tools version 0.9-rc3" ;; *) bad "version: $v" ;; esac

avahi-publish-service -s "QNX Test Service" _qnxtest._tcp 4242 "board=qemu" "sdp=8.0" > pub.log 2>&1 &
PIDS="$PIDS $!"
i=0; while ! grep -q 'Established under name' pub.log && [ $i -lt 40 ]; do sleep 0.5; i=$((i+1)); done
grep -q 'Established under name' pub.log && ok "avahi-publish-service: service established" || bad "publish: $(cat pub.log | tr '\n' ' ')"

timeout 30 avahi-browse -p -r -t _qnxtest._tcp > browse.log 2>&1
# -p (parsable) escapes the spaces of the name as \032; printf, not echo:
# ksh's echo would expand those backslash sequences.
grep '^=' browse.log | grep -F 'QNX\032Test\032Service' | head -n 1 > res.line
printf '  info resolved: %s\n' "$(cat res.line)"
grep -F -q ";QNX\\032Test\\032Service;_qnxtest._tcp;local;$HN.local;127.0.0.1;4242;" res.line \
  && ok "avahi-browse -r: QNX Test Service at $HN.local 127.0.0.1:4242" \
  || { bad "browse/resolve:"; sed 's/^/       /' browse.log | head -5; }
grep -F -q '"board=qemu"' res.line && grep -F -q '"sdp=8.0"' res.line && ok "TXT record board=qemu sdp=8.0" || bad "TXT record"

a=$(timeout 20 avahi-resolve -4 -n $HN.local 2>&1)
echo "  info avahi-resolve -n: $a"
case "$a" in "$HN.local"*127.0.0.1*) ok "own host name resolves to 127.0.0.1 over mDNS" ;; *) bad "resolve -n: $a" ;; esac
a=$(timeout 20 avahi-resolve -a 127.0.0.1 2>&1)
case "$a" in *"$HN.local"*) ok "avahi-resolve -a 127.0.0.1 -> $HN.local" ;; *) bad "resolve -a: $a" ;; esac

avahi-publish-address -R qnx-extra-$$.local 127.0.0.1 > addr.log 2>&1 &
PIDS="$PIDS $!"
i=0; while ! grep -q 'Established under name' addr.log && [ $i -lt 40 ]; do sleep 0.5; i=$((i+1)); done
a=$(timeout 20 avahi-resolve -4 -n qnx-extra-$$.local 2>&1)
case "$a" in *127.0.0.1*) ok "published address record qnx-extra-$$.local resolves" ;; *) bad "publish-address: $a $(cat addr.log)" ;; esac

stop_all
grep -q 'exiting' daemon.log && ok "avahi-daemon exited cleanly on SIGTERM" || bad "avahi-daemon exit: $(tail -2 daemon.log | tr '\n' ' ')"
[ -f $RUNDIR/pid ] && bad "PID file left behind" || ok "PID file removed"
grep -q 'Got SIGTERM, quitting' pub.log && ok "avahi-publish-service quit cleanly on SIGTERM" || bad "avahi-publish-service on SIGTERM: $(tail -2 pub.log | tr '\n' ' ')"

cleanup; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS avahi"; exit 0; }
echo "FAIL avahi ($fail check(s) failed)"; exit 1
