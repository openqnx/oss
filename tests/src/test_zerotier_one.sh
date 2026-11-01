#!/bin/sh
# ZeroTier One without a network: zerotier-idtool with a fixed identity
# (known public part and address, a deterministic Ed25519 signature of a
# known file, verify accepting it and rejecting another file, validate
# rejecting a corrupted key) and a freshly generated one; then the service
# itself, zerotier-one on a temporary home directory and port (port mapping
# off), queried over its local HTTP API by zerotier-cli: `info` must report
# the fixed identity's address and 1.16.1, `listnetworks` no networks.
# Run the test itself in the background, its output to a log, and wait at
# most 170 s: a hang ends as a FAIL with whatever the test printed so far.
if [ -z "$QNXPORT_INNER" ]; then
  QNXPORT_INNER=/tmp/qnxport-out.$$; export QNXPORT_INNER
  sh "$0" "$@" > "$QNXPORT_INNER" 2>&1 < /dev/null &
  qp=$!; qi=0
  while kill -0 $qp 2>/dev/null && [ $qi -lt 170 ]; do sleep 1; qi=$((qi+1)); done
  if kill -0 $qp 2>/dev/null; then
    for p in $(cat "$QNXPORT_INNER.pids" 2>/dev/null); do kill $p 2>/dev/null; done
    kill $qp 2>/dev/null; sleep 1; kill -KILL $qp 2>/dev/null
    cat "$QNXPORT_INNER"; echo "  FAIL watchdog: still running after 170 s -- stopped"
    echo "FAIL ${0##*/} (watchdog)"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit 1
  fi
  wait $qp; rc=$?; cat "$QNXPORT_INNER"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit $rc
fi
N=zerotier-one
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/sbin" ] || L=/data/oss/usr/local
ZO=$L/sbin/zerotier-one; ZI=$L/sbin/zerotier-idtool; ZC=$L/sbin/zerotier-cli
T=$(mktemp -d /tmp/qnxport-zt.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
# Whatever the test starts stops with it, on every exit path; the outer
# shell above kills it on a hang.
QP=$QNXPORT_INNER.pids; : > "$QP"
S=
stopall() {
  if [ -n "$S" ]; then
    kill "$S" 2>/dev/null
    i=0; while kill -0 "$S" 2>/dev/null && [ $i -lt 10 ]; do sleep 1; i=$((i+1)); done
    kill -9 "$S" 2>/dev/null; wait "$S" 2>/dev/null; S=
  fi
  : > "$QP"; [ -n "$T" ] && [ -d "$T" ] && { cd /; rm -rf "$T"; }
}
trap stopall EXIT
trap 'exit 143' TERM INT HUP

V=$("$ZO" -v 2>&1); info "zerotier-one -v: $V"
[ "$V" = "1.16.1" ] && ok "zerotier-one -v: 1.16.1" || bad "zerotier-one -v: $V"

# A fixed test identity (made for this test only) and its known answers.
ADDR=3829c0566b
PUB=3829c0566b:0:aee98c168ff486199fdd5fe98a0abeeb0aa6f0f12484f9d051842c8fc6d539341408d8e2e067d556a00c3aa03c9b0819fb08ecdbc8dcd26caee2be8065e4d89a
SEC=$PUB:2b35d99b8f37a8bd3a4ece676d658511d00a0cffdfc14b16645c425289a87a0be594a3dd177141de015435e396f6c428fb4d77c105ec0166c9927a76eb177c4c
SIG=dbd0f4d706638d19607b8ca87e85d2b09c58b71443701c6a6fe01203f7cc724ec0d9b930a26edc3553c8178eb0d6a6bd1e0bc67f28064609cf27983cc9783b06da77aabaeae58334ca6b83ce67f24ae33aa08bd36677ef285e79dcd44803d058
printf '%s' "$SEC" > id.secret
printf 'QNX port test: the quick brown fox\n' > msg.txt
printf 'QNX port test: the quick brown cat\n' > other.txt

r=$("$ZI" getpublic id.secret 2>&1)
[ "$r" = "$PUB" ] && ok "getpublic: the known public identity (address $ADDR)" || bad "getpublic: $r"
printf '%s' "$PUB" > id.public
r=$("$ZI" validate id.public 2>&1); rc=$?
[ $rc -eq 0 ] && ok "validate: the public identity is valid" || bad "validate: rc=$rc $r"
printf '%s' "$PUB" | sed 's/:aee9/:bee9/' > bad.public
r=$("$ZI" validate bad.public 2>&1); rc=$?
[ $rc -ne 0 ] && ok "validate: a corrupted key fails (rc=$rc)" || bad "validate accepted a corrupted key: $r"
r=$("$ZI" sign id.secret msg.txt 2>&1)
[ "$r" = "$SIG" ] && ok "sign: the known Ed25519 signature of msg.txt" || bad "sign: $r"
r=$("$ZI" verify id.public msg.txt "$SIG" 2>&1); rc=$?
[ $rc -eq 0 ] && ok "verify: signature valid for msg.txt" || bad "verify: rc=$rc $r"
r=$("$ZI" verify id.public other.txt "$SIG" 2>&1); rc=$?
[ $rc -ne 0 ] && ok "verify: the same signature fails for another file" || bad "verify accepted another file: $r"

# A new identity: the hashcash-style address derivation runs here.
r=$("$ZI" generate new.secret new.public 2>&1); rc=$?
if [ $rc -eq 0 ] && [ -s new.secret ] && [ -s new.public ]; then
  a=$(cut -d: -f1 new.public)
  info "generated identity $a"
  echo "$a" | grep -E -q '^[0-9a-f]{10}$' && ok "generate: a 10-hex-digit address" || bad "generate: address $a"
  [ "$("$ZI" getpublic new.secret 2>&1)" = "$(cat new.public)" ] && ok "generate: getpublic of the new secret is the new public" || bad "generate: public mismatch"
  "$ZI" validate new.secret >/dev/null 2>&1 && ok "generate: the new identity validates" || bad "generate: new identity invalid"
else
  bad "generate: rc=$rc $r"
fi

# The service, on its own home directory with the fixed identity.
H=$T/home; mkdir "$H"; cp id.secret "$H/identity.secret"; cp id.public "$H/identity.public"
cat > "$H/local.conf" <<EOF
{"settings":{"portMappingEnabled":false,"allowSecondaryPort":false,"allowTcpFallbackRelay":false}}
EOF
P=$((29100 + $$ % 800))
"$ZO" -U -p$P "$H" > service.log 2>&1 &
S=$!; echo $S >> "$QP"
info "zerotier-one pid $S on port $P"
i=0; r=
while [ $i -lt 20 ]; do
  r=$("$ZC" -D"$H" -p$P info 2>&1) && break
  kill -0 $S 2>/dev/null || break
  i=$((i+1)); sleep 1
done
case "$r" in
  "200 info $ADDR 1.16.1 "*) ok "zerotier-cli info: $r" ;;
  *) bad "zerotier-cli info: $r"; info "service log: $(head -c 400 service.log)" ;;
esac
[ -s "$H/authtoken.secret" ] && ok "the service wrote its API token (authtoken.secret)" || bad "no authtoken.secret"
r=$("$ZC" -D"$H" -p$P -j info 2>&1)
echo "$r" | grep -q "\"publicIdentity\": \"$PUB\"" && ok "zerotier-cli -j info: JSON with the public identity" || bad "-j info: $(echo "$r" | head -c 300)"
r=$("$ZC" -D"$H" -p$P listnetworks 2>&1)
[ "$(echo "$r" | wc -l | tr -d ' ')" = 1 ] && echo "$r" | grep -q "^200 listnetworks" && ok "listnetworks: no networks joined" || bad "listnetworks: $r"
r=$("$ZC" -D"$H" -p$P listpeers 2>&1)
echo "$r" | head -n 1 | grep -q "^200 listpeers" && ok "listpeers answered ($(($(echo "$r" | wc -l) - 1)) peers known: the planet's roots)" || bad "listpeers: $r"
r=$("$ZC" -p$P -Tnot-the-token info 2>&1); rc=$?
case "$r" in *401*) ok "a wrong API token is refused (401)" ;; *) bad "wrong token: rc=$rc $r" ;; esac

stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
