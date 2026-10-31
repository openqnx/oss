#!/bin/sh
# sscep (SCEP client) against a one-shot SCEP responder on loopback: the
# test makes a CA certificate with openssl, BusyBox nc serves it as the
# GetCACert reply, and sscep getca must fetch it over HTTP, print its
# SHA-256 fingerprint (equal to openssl's) and write it out as PEM with the
# right subject. Also: the usage text, and a clean error with no server.
# Run the test itself in the background, its output to a log, and wait at
# most 170 s: nothing it leaves behind can hold the runner's output, and a
# hang ends as a FAIL with whatever the test printed so far.
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
N=sscep
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
SC=$L/bin/sscep
T=$(mktemp -d /tmp/qnxport-$N.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
# Whatever the test starts stops with it, on every exit path (track PID;
# qstop_hook for anything else); the outer shell above kills them on a hang.
QP=$QNXPORT_INNER.pids; : > "$QP"
track() { echo "$1" >> "$QP"; }
qstop_hook() { :; }
stopall() {
  qstop_hook 2>/dev/null
  for p in $(cat "$QP" 2>/dev/null); do kill "$p" 2>/dev/null; done
  : > "$QP"; [ -n "$T" ] && [ -d "$T" ] && { cd /; rm -rf "$T"; }
}
trap stopall EXIT
trap 'exit 143' TERM
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
r=$("$SC" 2>&1); case "$r" in *"sscep version "*) ok "usage: $(echo "$r" | awk '/sscep version/ {print; exit}')" ;; *) bad "usage: $(echo "$r" | head -n 3)" ;; esac
OSSL=$(command -v openssl)
[ -n "$OSSL" ] || { bad "openssl not found (needed to make the test CA)"; echo "FAIL $N ($fail check(s) failed)"; exit 1; }
# A config of its own: the openssl found may have no openssl.cnf installed.
printf '[req]\ndistinguished_name = dn\nx509_extensions = v3\nprompt = no\n[dn]\nCN = QNX SCEP Test CA\nO = QNX Port Tests\n[v3]\nbasicConstraints = critical,CA:TRUE\nkeyUsage = keyCertSign,cRLSign\n' > ca.cnf
to 60 "$OSSL" req -config ca.cnf -x509 -newkey rsa:2048 -nodes -keyout ca.key -days 2 -outform DER -out ca.der > o.log 2>&1 \
  && ok "test CA made with $("$OSSL" version 2>&1 | cut -d' ' -f1-2)" || { bad "openssl req"; head -3 o.log; }
want=$("$OSSL" x509 -inform DER -in ca.der -noout -fingerprint -sha256 2>/dev/null | sed 's/.*=//')
P=$((18100 + $$ % 2000))
NC=; if [ -x "$L/bin/busybox" ]; then NC="$L/bin/busybox nc"; elif command -v nc >/dev/null 2>&1; then NC=nc; fi
if [ -n "$NC" ] && [ -f ca.der ]; then
  # sscep makes two requests (GetCACaps, then GetCACert), one connection
  # each: a persistent nc (-ll) runs this responder for every connection.
  printf 'HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nPOSTPKIOperation\nSHA-256\nAES\n' > caps.bin
  { printf 'HTTP/1.0 200 OK\r\nContent-Type: application/x-x509-ca-cert\r\nContent-Length: %d\r\n\r\n' "$(wc -c < ca.der)"; cat ca.der; } > reply.bin
  printf '#!/bin/sh\nread -r req\necho "$req" >> %s/requests.txt\ncase "$req" in *GetCACaps*) cat %s/caps.bin ;; *) cat %s/reply.bin ;; esac\n' "$T" "$T" "$T" > responder.sh
  chmod +x responder.sh
  $NC -ll -p $P -e "$T/responder.sh" > nc.out 2> nc.err &
  S=$!; track $S; sleep 1
  r=$(to 30 "$SC" getca -u "http://127.0.0.1:$P/scep/pkiclient.exe" -c "$T/got-ca.crt" -F sha256 -v 2>&1)
  kill $S 2>/dev/null; wait $S 2>/dev/null
  awk '/^GET .*operation=GetCACaps/ {a=1} /^GET .*operation=GetCACert/ {b=1} END {exit !(a && b)}' requests.txt 2>/dev/null && ok "sscep sent GetCACaps, then GetCACert" || { bad "requests: $(cat requests.txt 2>/dev/null | tr '\r\n' '  ')"; cat nc.err; }
  got=$(echo "$r" | awk '/fingerprint:/ {sub(/.*fingerprint: */, ""); print; exit}')
  [ -n "$want" ] && [ "$got" = "$want" ] && ok "fingerprint matches openssl: ${got%%:*}:..." || bad "fingerprint: '$got' want '$want'"
  s=$("$OSSL" x509 -in got-ca.crt -noout -subject 2>&1)
  case "$s" in *"QNX SCEP Test CA"*) ok "CA written as PEM: $s" ;; *) bad "written CA: $s"; echo "$r" | tail -5 ;; esac
else bad "no CA or no nc (BusyBox) to serve the SCEP replies"; fi
r=$(to 30 "$SC" getca -u "http://127.0.0.1:1/scep" -c "$T/none.crt" 2>&1); rc=$?
[ $rc -ne 0 ] && [ ! -f none.crt ] && ok "no server: fails cleanly (rc=$rc)" || bad "no server: rc=$rc $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
