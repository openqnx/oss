#!/bin/sh
# ntttcp on loopback: a receiver (-r) and a sender (-s) on 127.0.0.1 with
# ntttcp's sync connection. TCP, 2 ports x 2 threads for 2 s with the
# sender pinned to CPU 0 (-m 2,0,...): both sides exit 0 on their own, the
# receiver got bytes and never more than the sender sent, both report the
# same CPU count, and the JSON/XML result files name this host and the
# parameters. UDP with a 8 Mbit/s limit (-B 8M) for 2 s: the receiver's
# byte count is in the range the limit allows (about 2 MB), and the
# receiver exits by itself.
echo "== ntttcp-for-linux =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-ntttcp.XXXXXX) || { echo "FAIL ntttcp-for-linux (mktemp)"; exit 1; }
RPID=
# Stop the receiver this test started -- on every exit path, also when the
# test is interrupted or killed by a timeout -- and remove the scratch dir.
stop_receiver() {
  [ -n "$RPID" ] || return 0
  kill $RPID 2>/dev/null
  i=0; while kill -0 $RPID 2>/dev/null && [ $i -lt 5 ]; do sleep 1; i=$((i+1)); done
  kill -9 $RPID 2>/dev/null; wait $RPID 2>/dev/null
  RPID=
}
cleanup() { stop_receiver; cd /; rm -rf "$T"; }
trap cleanup EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

# Waits up to <n> s for the receiver to exit by itself; its exit status in $rrc.
wait_receiver() {
  i=0; while kill -0 $RPID 2>/dev/null && [ $i -lt $1 ]; do sleep 1; i=$((i+1)); done
  if kill -0 $RPID 2>/dev/null; then rrc=timeout; stop_receiver; return 1; fi
  wait $RPID; rrc=$?; RPID=; return 0
}
# "total bytes	:<n>" from a log.
total() { sed -n 's/.*total bytes[^:]*:\([0-9][0-9]*\).*/\1/p' "$1" | head -n 1; }
cores() { sed -n 's/.*cpu cores[^:]*:\([0-9][0-9]*\).*/\1/p' "$1" | head -n 1; }

V=$(ntttcp -h 2>&1 | head -n 1)
case "$V" in "NTTTCP for Linux 1.4.0"*) ok "ntttcp -h: $V" ;; *) bad "ntttcp -h: $V" ;; esac

PORT=$((15601 + ($$ % 100) * 4))
# The sync connection uses PORT-1.
ntttcp -r127.0.0.1 -P 2 -p $PORT -t 2 -j r.json -x r.xml > r.log 2>&1 &
RPID=$!
sleep 1
ntttcp -s127.0.0.1 -m 2,0,127.0.0.1 -n 2 -p $PORT -t 2 -j s.json > s.log 2>&1; src=$?
wait_receiver 20
[ $src -eq 0 ] && ok "TCP sender exited 0" || bad "TCP sender: rc=$src $(grep -E 'ERR' s.log | head -n 3)"
[ "$rrc" = 0 ] && ok "TCP receiver finished the test and exited 0 by itself" || bad "TCP receiver: rc=$rrc"
grep -q "main: set cpu affinity" s.log && ! grep -q "cannot set cpu affinity" s.log \
  && ok "sender pinned to CPU 0 (-m 2,0,...)" || bad "cpu affinity: $(grep -i affinity s.log)"
st=$(total s.log); rt=$(total r.log)
echo "  info TCP: sender $st bytes, receiver $rt bytes; $(grep throughput r.log | tail -n 1 | tr -s '\t ' ' ')"
if [ -n "$st" ] && [ -n "$rt" ] && [ "$rt" -gt 0 ] && [ "$rt" -le "$st" ]; then
  ok "TCP: the receiver got data, never more than the sender sent"
else
  bad "TCP: sender '$st' receiver '$rt'"
fi
grep -q "4 connections tested" s.log && ok "TCP: 4 connections tested (2 ports x 2 threads)" || bad "TCP: $(grep connections s.log)"
sc=$(cores s.log); rc=$(cores r.log)
[ -n "$sc" ] && [ "$sc" -ge 1 ] && [ "$sc" = "$rc" ] && ok "both sides report $sc CPU core(s)" || bad "cpu cores: sender '$sc' receiver '$rc'"
grep -E -q "ERR" r.log s.log && echo "  info errors logged: $(grep -h ERR r.log s.log | head -n 3 | tr '\n' ' ')"
H=$(hostname)
grep -q "\"computername\" : \"$H\"" r.json && ok "JSON: computername is $H" || bad "JSON: $(grep computername r.json)"
grep -q "\"port\" : \"$PORT\"" r.json && grep -q "\"runTime\" : \"2\"" r.json \
  && ok "JSON: port $PORT, runTime 2" || bad "JSON: parameters $(grep -E '"(port|runTime)"' r.json | tr -d '\n')"
mb=$(sed -n '/"totalBytes"/,/}/p' r.json | sed -n 's/.*"value" : "\([0-9.]*\)".*/\1/p' | head -n 1)
# MB with 6 decimals = the byte count / 10^6.
[ -n "$mb" ] && [ "$(echo "$mb" | tr -d .)" -eq "$rt" ] 2>/dev/null && ok "JSON: totalBytes $mb MB = $rt bytes" \
  || bad "JSON: totalBytes '$mb' vs $rt"
grep -q "<ntttcpr" r.xml && grep -q "computername=\"$H\"" r.xml && ok "XML: ntttcpr result for $H" || bad "XML: $(head -c 200 r.xml)"

ntttcp -r127.0.0.1 -u -P 1 -p $((PORT+2)) -t 2 > ur.log 2>&1 &
RPID=$!
sleep 1
ntttcp -s127.0.0.1 -u -P 1 -n 1 -p $((PORT+2)) -t 2 -b 1K -B 8M > us.log 2>&1; src=$?
# A UDP receiver thread notices the end of the test when its blocking
# receive times out, after ntttcp's 30 s socket timeout.
wait_receiver 45
ut=$(total ur.log)
echo "  info UDP: sender $(total us.log) bytes, receiver $ut bytes"
[ $src -eq 0 ] && [ "$rrc" = 0 ] && ok "UDP: sender and receiver exited 0" || bad "UDP: sender rc=$src receiver rc=$rrc"
# 8 Mbit/s for 2 s is 2,000,000 bytes; allow for a slow target and for timer slack.
[ -n "$ut" ] && [ "$ut" -ge 200000 ] && [ "$ut" -le 2600000 ] && ok "UDP: -B 8M limited 2 s to $ut bytes (<= 2.6 MB)" \
  || bad "UDP: receiver bytes '$ut'"

cleanup; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS ntttcp-for-linux"; exit 0; }
echo "FAIL ntttcp-for-linux ($fail check(s) failed)"; exit 1
