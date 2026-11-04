#!/bin/sh
# lldpd in the foreground on a control socket of its own (-u), limited to an
# interface that does not exist (-I) so it sends nothing on the target's
# network. Its start-up configuration (-O, applied by the lldpcli it starts)
# sets the system name and description; lldpcli then has to report them in
# the local chassis (JSON) and the running configuration (key/value), change
# a setting at run time, and lldpd has to stop cleanly on SIGTERM, removing
# its socket. The daemon is stopped on every exit path.
echo "== lldpd =="
fail=0
# The programs are in sbin/, which is not on every PATH.
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
PATH=$L/sbin:$L/bin:$PATH; export PATH
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-lldpd.XXXXXX) || { echo "FAIL lldpd (mktemp)"; exit 1; }
DPID=
stop_daemon() {
  [ -n "$DPID" ] || return 0
  kill $DPID 2>/dev/null
  i=0; while kill -0 $DPID 2>/dev/null && [ $i -lt 5 ]; do sleep 1; i=$((i+1)); done
  kill -9 $DPID 2>/dev/null; wait $DPID 2>/dev/null
  DPID=
}
cleanup() { stop_daemon; cd /; rm -rf "$T"; }
trap cleanup EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

V=$(lldpd -v 2>&1); [ "$V" = "1.0.22" ] && ok "lldpd -v: 1.0.22" || bad "lldpd -v: $V"
V=$(lldpcli -v 2>&1); [ "$V" = "1.0.22" ] && ok "lldpcli -v: 1.0.22" || bad "lldpcli -v: $V"
lldpd -vv > vv.txt 2>&1
grep -q "Privilege separation: *disabled" vv.txt && ok "lldpd -vv: built without privilege separation" \
  || bad "lldpd -vv: $(grep -i privilege vv.txt)"

SOCK=$T/lldpd.socket
cat > lldpd.conf <<'EOF'
configure system hostname qnxtest-lldp
configure system description "QNX port test"
configure lldp tx-interval 45
EOF
CLI=$(command -v lldpcli)
lldpd -dd -u "$SOCK" -O "$T/lldpd.conf" -L "$CLI" -I qnxnone0 > daemon.log 2>&1 &
DPID=$!
echo "  info lldpd pid $DPID, socket $SOCK"

# The configuration is applied by an lldpcli that lldpd starts: wait for it.
i=0; got=
while [ $i -lt 20 ]; do
  lldpcli -u "$SOCK" -f keyvalue show configuration > conf.txt 2> conf.err \
    && grep -q "^configuration.config.hostname=qnxtest-lldp$" conf.txt && { got=1; break; }
  kill -0 $DPID 2>/dev/null || break
  i=$((i+1)); sleep 1
done
[ -n "$got" ] && ok "start-up configuration applied (after ${i}s)" \
  || bad "configuration not applied: $(head -c 300 conf.err) $(tail -n 5 daemon.log | tr '\n' ' ')"
[ -S "$SOCK" ] && ok "control socket created at -u path" || bad "no control socket $SOCK"
grep -q "^configuration.config.description=QNX port test$" conf.txt \
  && ok "keyvalue: description=QNX port test" || bad "description: $(grep description conf.txt)"
grep -q "^configuration.config.tx-delay=45$" conf.txt \
  && ok "keyvalue: tx-interval 45 -> tx-delay=45" || bad "tx-delay: $(grep tx-delay conf.txt)"
grep -q "^configuration.config.iface-pattern=qnxnone0$" conf.txt \
  && ok "keyvalue: iface-pattern=qnxnone0 (-I)" || bad "iface-pattern: $(grep iface-pattern conf.txt)"

lldpcli -u "$SOCK" -f json show chassis > chassis.json 2>&1
grep -q '"qnxtest-lldp": {' chassis.json && grep -q '"descr": "QNX port test"' chassis.json \
  && ok "json: local chassis named qnxtest-lldp, descr QNX port test" \
  || bad "chassis json: $(head -c 300 chassis.json)"

lldpcli -u "$SOCK" configure system platform QNXTEST > plat.txt 2>&1; rc=$?
lldpcli -u "$SOCK" -f keyvalue show configuration > conf2.txt 2>&1
[ $rc -eq 0 ] && grep -q "^configuration.config.platform=QNXTEST$" conf2.txt \
  && ok "run-time change: platform=QNXTEST" || bad "platform: rc=$rc $(cat plat.txt) $(grep platform conf2.txt)"

lldpcli -u "$SOCK" -f keyvalue show neighbors > nb.txt 2>&1; rc=$?
[ $rc -eq 0 ] && [ ! -s nb.txt ] && ok "no neighbours (no interface in use)" || bad "neighbors: rc=$rc $(head -c 200 nb.txt)"

lldpcli -u "$T/nosuch.socket" show configuration > none.txt 2>&1; rc=$?
[ $rc -ne 0 ] && grep -q "unable to connect to socket" none.txt \
  && ok "lldpcli without a daemon: refused" || bad "no daemon: rc=$rc $(head -c 200 none.txt)"

if ! kill -0 $DPID 2>/dev/null; then
  wait $DPID 2>/dev/null; rc=$?; DPID=
  bad "lldpd exited before SIGTERM (rc=$rc)"
else kill $DPID 2>/dev/null
i=0; while kill -0 $DPID 2>/dev/null && [ $i -lt 10 ]; do sleep 1; i=$((i+1)); done
if kill -0 $DPID 2>/dev/null; then bad "lldpd still running 10 s after SIGTERM"
else wait $DPID 2>/dev/null; DPID=; ok "lldpd stopped on SIGTERM"
     [ ! -e "$SOCK" ] && ok "control socket removed at exit" || bad "control socket left behind"
fi
fi
stop_daemon
echo "  info daemon log: $(grep -v "^ *$" daemon.log | head -n 3 | tr '\n' ' ')"

cleanup; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS lldpd"; exit 0; }
echo "FAIL lldpd ($fail check(s) failed)"; exit 1
