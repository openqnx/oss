#!/bin/sh
# socat 1.8.0.3, loopback only: the version and its features; a TCP relay
# (one connection through SYSTEM:"tr a-z A-Z", answer known) and a 1 MiB
# file through a TCP relay compared byte for byte; a UNIX socket relay; UDP
# datagrams to a receiver writing a file; OpenSSL (TLS) between socat's
# own server and client with a fixed self-signed certificate (the client
# pins it with cafile=, so the handshake is verified); EXEC with a pty;
# procan and filan. Every relay socat this test starts is killed (by PID)
# on every exit path.
echo "== socat =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-socat.XXXXXX) || { echo "FAIL socat (mktemp)"; exit 1; }
PIDS=
cleanup() {
  for p in $PIDS; do kill $p 2>/dev/null; done
  for p in $PIDS; do i=0; while kill -0 $p 2>/dev/null && [ $i -lt 5 ]; do sleep 1; i=$((i+1)); done; kill -9 $p 2>/dev/null; done
  PIDS=; cd /; rm -rf "$T"
}
trap cleanup EXIT
trap 'echo "  FAIL interrupted"; exit 1' INT TERM HUP
cd "$T" || exit 1
# Ports from the PID, so that two runs do not collide.
B=$(( 20000 + ($$ % 2000) * 10 ))
# wait until a TCP port answers (or a UNIX socket exists)
waitport() { i=0; while [ $i -lt 15 ]; do socat -T1 /dev/null TCP:127.0.0.1:$1 2>/dev/null && return 0; sleep 1; i=$((i+1)); done; return 1; }
waitsock() { i=0; while [ $i -lt 15 ]; do [ -S "$1" ] && return 0; sleep 1; i=$((i+1)); done; return 1; }

V=$(socat -V 2>&1)
echo "$V" | grep -q "^socat version 1.8.0.3 " && ok "socat -V: version 1.8.0.3" || bad "socat -V: $(echo "$V" | sed -n 2p)"
echo "$V" | grep -q "#define WITH_OPENSSL 1" && ok "built with OpenSSL" || bad "WITH_OPENSSL missing"
echo "$V" | grep -q "#define WITH_READLINE 1" && ok "built with readline" || bad "WITH_READLINE missing"

# --- TCP relay through a filter ---
socat -T5 TCP-LISTEN:$B,bind=127.0.0.1,reuseaddr,fork SYSTEM:"tr a-z A-Z" 2> tcp.err &
PIDS="$PIDS $!"
if waitport $B; then
  r=$(echo "hello from qnx" | socat -T5 - TCP:127.0.0.1:$B 2>&1)
  [ "$r" = "HELLO FROM QNX" ] && ok "TCP relay through tr: HELLO FROM QNX" || bad "TCP relay: '$r' $(cat tcp.err)"
else bad "TCP listener on 127.0.0.1:$B never answered: $(cat tcp.err)"; fi

# --- 1 MiB through TCP into a file ---
P2=$((B+1))
dd if=/dev/urandom of=big.bin bs=1024 count=1024 2>/dev/null
socat -u -T10 TCP-LISTEN:$P2,bind=127.0.0.1,reuseaddr OPEN:big.out,creat,trunc 2> big.err &
BP=$!; PIDS="$PIDS $BP"
sleep 1
socat -u -T10 OPEN:big.bin TCP:127.0.0.1:$P2,retry=20,interval=1 2>> big.err
i=0; while kill -0 $BP 2>/dev/null && [ $i -lt 20 ]; do sleep 1; i=$((i+1)); done
cmp -s big.bin big.out && ok "1 MiB file through TCP: identical" || bad "1 MiB through TCP: $(ls -l big.out 2>&1) $(cat big.err)"

# --- UNIX socket ---
socat -T5 UNIX-LISTEN:$T/s.sock,fork SYSTEM:"tr a-z n-za-m" 2> unix.err &
PIDS="$PIDS $!"
if waitsock "$T/s.sock"; then
  r=$(echo "abcdef" | socat -T5 - UNIX-CONNECT:$T/s.sock 2>&1)
  [ "$r" = "nopqrs" ] && ok "UNIX socket relay through tr (rot13): nopqrs" || bad "UNIX relay: '$r' $(cat unix.err)"
else bad "UNIX socket $T/s.sock never appeared: $(cat unix.err)"; fi

# --- UDP ---
P3=$((B+2))
socat -u UDP4-RECV:$P3,bind=127.0.0.1 OPEN:udp.out,creat,append 2> udp.err &
UP=$!; PIDS="$PIDS $UP"
sleep 1
for n in 1 2 3; do echo "datagram $n" | socat -u - UDP4-SENDTO:127.0.0.1:$P3; done
sleep 1
r=$(cat udp.out 2>/dev/null | tr '\n' ' ')
[ "$r" = "datagram 1 datagram 2 datagram 3 " ] && ok "UDP: 3 datagrams received in order" || bad "UDP: '$r' $(cat udp.err)"

# --- OpenSSL ---
cat > key.pem <<'PEM'
-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgHrjXAKERV/5LLfa8
kZlZxw1oKfO/pYgQSpMTreVQBXuhRANCAAQBhimwKZ6py0V3q0rMEvmoU6b4TCh2
5Pj++bPX80yHtWnlYumiPZ3BrrN/dk3yvrmW0AZnDeW4y2zhigEgh7I3
-----END PRIVATE KEY-----
PEM
cat > cert.pem <<'PEM'
-----BEGIN CERTIFICATE-----
MIIBijCCAS+gAwIBAgIUYjWiq9tX4VB35TT7wZxZhDVF/EEwCgYIKoZIzj0EAwIw
GTEXMBUGA1UEAwwOc29jYXQucW54LnRlc3QwIBcNMjYxMTAxMDQxMjMwWhgPMjEy
NjEwMDgwNDEyMzBaMBkxFzAVBgNVBAMMDnNvY2F0LnFueC50ZXN0MFkwEwYHKoZI
zj0CAQYIKoZIzj0DAQcDQgAEAYYpsCmeqctFd6tKzBL5qFOm+EwoduT4/vmz1/NM
h7Vp5WLpoj2dwa6zf3ZN8r65ltAGZw3luMts4YoBIIeyN6NTMFEwHQYDVR0OBBYE
FL/H1AmHKPHlTkJ3bmQJKHxXkjqGMB8GA1UdIwQYMBaAFL/H1AmHKPHlTkJ3bmQJ
KHxXkjqGMA8GA1UdEwEB/wQFMAMBAf8wCgYIKoZIzj0EAwIDSQAwRgIhANYs8tl5
6Fz+SXSslyWVrI3BcUf06C6aZLzRwQGqlcxaAiEAzijUm8V355XzUawK6hXsi/9j
gWSRYqONueZFRz4hlbk=
-----END CERTIFICATE-----
PEM
P4=$((B+3))
socat -T5 OPENSSL-LISTEN:$P4,bind=127.0.0.1,reuseaddr,fork,cert=$T/cert.pem,key=$T/key.pem,verify=0 SYSTEM:"tr a-z A-Z" 2> ssl.err &
PIDS="$PIDS $!"
if waitport $P4; then
  r=$(echo "tls secret" | socat -T5 - OPENSSL:127.0.0.1:$P4,cafile=$T/cert.pem,commonname=socat.qnx.test 2> sslc.err)
  [ "$r" = "TLS SECRET" ] && ok "OpenSSL: verified TLS connection (pinned certificate), TLS SECRET" || bad "OpenSSL: '$r' $(cat sslc.err ssl.err | tail -n 4)"
  r=$(echo "x" | socat -T5 - OPENSSL:127.0.0.1:$P4,cafile=$T/cert.pem,commonname=other.name 2>&1)
  case "$r" in *X*) bad "OpenSSL: wrong common name accepted" ;; *) ok "OpenSSL: a wrong commonname is refused" ;; esac
else bad "OpenSSL listener never answered: $(cat ssl.err)"; fi

# --- EXEC on a pty ---
r=$(echo "pty line" | socat -T5 - EXEC:"cat",pty,raw,echo=0 2>&1)
[ "$r" = "pty line" ] && ok "EXEC:cat on a pseudo-terminal" || bad "EXEC pty: '$r'"

# --- procan, filan ---
procan > procan.out 2>&1 && grep -q "process id" procan.out && ok "procan: reports the process id" || bad "procan: $(head -n 3 procan.out)"
filan -i 0 < /dev/null > filan.out 2>&1 && grep -qi "chrdev\|character" filan.out && ok "filan: fd 0 (/dev/null) is a character device" || echo "  info filan: $(head -n 2 filan.out)"

[ $fail -eq 0 ] && { echo "PASS socat"; exit 0; } || { echo "FAIL socat ($fail check(s) failed)"; exit 1; }
