#!/bin/sh
# strongSwan 6.0.6:
#  1. libstrongswan_tests, strongSwan's own unit-test runner, on suites with
#     known answers: chunk/ASN.1/identity parsing, hashers and crypters, RSA
#     and ECDSA key generation, signing and verification with fixed keys, X.509
#     certificate encoding and parsing -- through the installed openssl, x509,
#     pem, pkcs1, pkcs8, pubkey, sha2 ... plugins.
#  2. exchange_tests: complete in-memory IKEv2 exchanges (IKE_SA_INIT,
#     IKE_AUTH, CHILD_SA create/rekey/delete) between two libcharon instances.
#  3. pki: an ECDSA CA key and self-signed CA certificate with a name
#     constraint, an RSA end-entity key and certificate issued by it, pki
#     --print with known subject/flags/key sizes, pki --verify trusted against
#     the CA and untrusted against another CA or when the constraint is broken.
#  4. If /var/run is writable: charon on loopback with a private config, and
#     swanctl talking to it over vici (version, stats, load and list a
#     connection and a PSK secret).
echo "== strongswan =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-strongswan.XXXXXX) || { echo "FAIL strongswan (mktemp)"; exit 1; }
# Whatever happens (a failed check, a signal, the runner timing out), stop the
# charon this test started -- by PID -- and remove what it created.
CPID=; MYPID=
cleanup() {
  if [ -n "$CPID" ]; then
    kill $CPID 2>/dev/null
    i=0; while kill -0 $CPID 2>/dev/null && [ $i -lt 10 ]; do sleep 1; i=$((i+1)); done
    kill -9 $CPID 2>/dev/null; wait $CPID 2>/dev/null
    CPID=
  fi
  [ -n "$MYPID" ] && [ -e /var/run/charon.pid ] && rm -f /var/run/charon.pid
  MYPID=
  cd /; rm -rf "$T"
}
trap cleanup EXIT
trap 'echo "  FAIL interrupted"; exit 1' INT TERM HUP
cd "$T" || exit 1
BIN=/usr/local/bin; CHARON=/usr/local/libexec/ipsec/charon
[ -x "$BIN/libstrongswan_tests" ] || BIN=$(dirname "$(command -v libstrongswan_tests 2>/dev/null || echo /x/x)")

# The runner prints per-suite lines and "Passed all N '<name>' suites".
# name = the runner's own name for its suite set ("libstrongswan",
# "exchanges"), which is what it prints in that line.
runsuites() { # name binary expected-count suites...
  name=$1; bin=$2; want=$3; shift 3
  if [ -n "$EXCL" ]; then
    TESTS_SUITES="$*" TESTS_REDUCED_KEYLENGTHS=1 TESTS_FUNCTIONS_EXCLUDE="$EXCL" "$bin" > "$name.out" 2>&1; rc=$?
  else
    TESTS_SUITES="$*" TESTS_REDUCED_KEYLENGTHS=1 "$bin" > "$name.out" 2>&1; rc=$?
  fi
  if [ $rc -eq 0 ] && grep -q "Passed all $want '$name' suites" "$name.out"; then
    ok "$name: all $want suites passed ($*)"
  else
    bad "$name: rc=$rc, want 'Passed all $want '$name' suites': $(grep -E "suites" "$name.out" | tail -n 1)"
    # the failing cases, with the runner's failure messages around them
    grep -E "Passed [0-9]+/|Failure|failed|FAIL|Warning|assert" "$name.out" | head -n 40 | sed 's/^/  info | /'
    tail -n 15 "$name.out" | sed 's/^/  info | /'
  fi
}
EXCL=
if [ -x "$BIN/libstrongswan_tests" ]; then
  # test_chunk_from_fd_huge (chunk suite) pushes 8192 4-byte send()s through
  # an AF_UNIX socketpair to a reader thread, every one a message pass to
  # io-sock, under strongSwan's fixed 2 s per-function limit
  # (TEST_FUNCTION_DEFAULT_TIMEOUT, no run-time override).  On the emulated
  # (TCG) aarch64 QEMU target that limit, not the code, ran out: SIGALRM
  # "timeout(14)".  Run it on its own, so the rest of the suite is judged
  # strictly and only a timeout of this one function is reported as info.
  EXCL=test_chunk_from_fd_huge
  runsuites libstrongswan "$BIN/libstrongswan_tests" 14 \
    "chunk,enum,identification,asn1,host,printf,hasher,crypter,rsa,ecdsa,certnames,serial_gen,serial_parse,signature params"
  echo "  info $(grep -m1 'loaded plugins' libstrongswan.out)"
  EXCL=
  TESTS_SUITES=chunk TESTS_FUNCTIONS=test_chunk_from_fd_huge "$BIN/libstrongswan_tests" > huge.out 2>&1; rc=$?
  if [ $rc -eq 0 ] && grep -q "Passed all 1 'libstrongswan' suites" huge.out; then
    ok "chunk_from_fd: 8192 messages over a socketpair read back in order"
  elif grep -q "test_chunk_from_fd_huge': timeout" huge.out; then
    echo "  info chunk_from_fd_huge hit strongSwan's 2 s per-test limit (slow emulated target, rc=$rc): not judged"
  else
    bad "chunk_from_fd_huge: rc=$rc: $(grep -E "Failure|suites" huge.out | tail -n 3 | tr '\n' ' ')"
  fi
else bad "libstrongswan_tests not found"; fi
if [ -x "$BIN/exchange_tests" ]; then
  runsuites exchanges "$BIN/exchange_tests" 4 "child create,ike rekey,child rekey,ike delete"
else bad "exchange_tests not found"; fi


# pki: keys and certificates with known contents.
PKI=$BIN/pki; [ -x "$PKI" ] || PKI=$(command -v pki 2>/dev/null)
if [ -z "$PKI" ] || [ ! -x "$PKI" ]; then bad "pki not installed"
else
  v=$("$PKI" --help 2>&1 | head -n 1); echo "  info pki: $v"
  case "$v" in *"strongSwan 6.0.6 PKI tool"*) ok "pki --help: strongSwan 6.0.6 PKI tool" ;; *) bad "pki --help: $v" ;; esac
  CADN="C=CH, O=QNX Test, CN=QNX Test CA"
  EEDN="C=CH, O=QNX Test, CN=moon.qnx.test"
  "$PKI" --gen --type ecdsa --size 256 --outform pem > ca.key 2>gen.err && grep -q "BEGIN EC PRIVATE KEY" ca.key \
    && ok "pki --gen: ECDSA 256 key (PEM)" || bad "pki --gen ecdsa: $(cat gen.err)"
  out=$("$PKI" --print --type ecdsa --in ca.key 2>&1)
  echo "$out" | grep -q "privkey: *ECDSA 256 bits" && ok "pki --print key: ECDSA 256 bits" || bad "pki --print ecdsa key: $out"
  "$PKI" --self --ca --lifetime 30 --serial 0102030405 --in ca.key --type ecdsa --dn "$CADN" \
     --nc-permitted qnx.test --outform pem > ca.crt 2>self.err && grep -q "BEGIN CERTIFICATE" ca.crt \
    && ok "pki --self: CA certificate" || bad "pki --self: $(cat self.err)"
  out=$("$PKI" --print --in ca.crt 2>&1)
  echo "$out" | grep -q "subject: *\"$CADN\"" && ok "CA subject: $CADN" || bad "CA subject: $out"
  echo "$out" | grep -q "issuer: *\"$CADN\"" && ok "CA issuer = subject" || bad "CA issuer: $out"
  echo "$out" | grep -q "serial: *01:02:03:04:05" && ok "CA serial 01:02:03:04:05" || bad "CA serial: $out"
  f=$(echo "$out" | grep "flags:")
  case "$f" in *"CA "*self-signed*) ok "CA flags:$(echo "$f" | sed 's/.*flags://')" ;; *) bad "CA flags: $f" ;; esac
  echo "$out" | grep -q "pubkey: *ECDSA 256 bits" && ok "CA pubkey: ECDSA 256 bits" || bad "CA pubkey: $out"
  [ "$(echo "$out" | sed -n '/permitted nameConstraints:/{n;p;}' | tr -d ' ')" = qnx.test ] && ok "CA name constraint: permitted qnx.test" || bad "CA permitted name constraint: $out"

  "$PKI" --gen --type rsa --size 2048 --outform pem > ee.key 2>gen.err && ok "pki --gen: RSA 2048 key" || bad "pki --gen rsa: $(cat gen.err)"
  "$PKI" --pub --in ee.key --outform pem > ee.pub 2>pub.err && grep -q "BEGIN PUBLIC KEY" ee.pub \
    && ok "pki --pub: public key extracted" || bad "pki --pub: $(cat pub.err)"
  issue() { # out dn san
    "$PKI" --issue --in ee.pub --type pub --cacert ca.crt --cakey ca.key --dn "$2" --san "$3" \
      --flag serverAuth --lifetime 10 --serial 0a0b0c --outform pem > "$1" 2>issue.err
  }
  issue ee.crt "$EEDN" moon.qnx.test && grep -q "BEGIN CERTIFICATE" ee.crt \
    && ok "pki --issue: end-entity certificate" || bad "pki --issue: $(cat issue.err)"
  out=$("$PKI" --print --in ee.crt 2>&1)
  echo "$out" | grep -q "subject: *\"$EEDN\"" && ok "EE subject: $EEDN" || bad "EE subject: $out"
  echo "$out" | grep -q "issuer: *\"$CADN\"" && ok "EE issuer: the CA" || bad "EE issuer: $out"
  echo "$out" | grep -q "altNames: *moon.qnx.test" && ok "EE altNames: moon.qnx.test" || bad "EE altNames: $out"
  f=$(echo "$out" | grep "flags:")
  case "$f" in *serverAuth*) case "$f" in *"CA "*) bad "EE flags include CA: $f" ;; *) ok "EE flags: serverAuth, not CA" ;; esac ;;
    *) bad "EE flags: $f" ;; esac
  echo "$out" | grep -q "pubkey: *RSA 2048 bits" && ok "EE pubkey: RSA 2048 bits" || bad "EE pubkey: $out"

  # pki --verify: the verdict on stdout, the chain it built on stderr.
  out=$("$PKI" --verify --in ee.crt --cacert ca.crt 2>v.err); rc=$?
  [ $rc -eq 0 ] && [ "$out" = "certificate trusted, lifetimes valid" ] && grep -q "reached self-signed root ca" v.err \
    && ok "pki --verify: $out" || bad "pki --verify: rc=$rc $out $(cat v.err)"
  out=$("$PKI" --verify --in ca.crt --cacert ca.crt 2>v.err); rc=$?
  [ $rc -eq 0 ] && ok "pki --verify: CA against itself trusted" || bad "pki --verify CA: rc=$rc $out"
  # Another CA (same DN, other key): the signature does not verify.
  "$PKI" --gen --type ecdsa --size 256 > other.key 2>/dev/null
  "$PKI" --self --ca --in other.key --type ecdsa --dn "$CADN" --outform pem > other.crt 2>/dev/null
  out=$("$PKI" --verify --in ee.crt --cacert other.crt 2>v.err); rc=$?
  [ $rc -eq 1 ] && [ "$out" = "certificate untrusted" ] \
    && ok "pki --verify against another CA: untrusted (rc 1)" || bad "pki --verify other CA: rc=$rc $out $(cat v.err)"
  # A subjectAltName outside the CA's permitted name space: the constraints
  # plugin must reject the chain.
  issue bad.crt "C=CH, O=QNX Test, CN=evil.example.com" evil.example.com
  out=$("$PKI" --verify --in bad.crt --cacert ca.crt 2>v.err); rc=$?
  [ $rc -eq 1 ] && [ "$out" = "certificate untrusted" ] && grep -q "does not match any permitted name constraints" v.err \
    && ok "pki --verify: SAN outside the CA's name constraint untrusted (constraints plugin)" \
    || bad "pki --verify name constraint: rc=$rc $out $(cat v.err)"
fi

# charon + swanctl over vici, with a private configuration.
if [ ! -x "$CHARON" ]; then bad "$CHARON not installed"
elif ! { [ -d /var/run ] && [ -w /var/run ]; }; then
  echo "  info /var/run is not writable: charon (fixed pidfile /var/run/charon.pid) not exercised"
elif [ -e /var/run/charon.pid ]; then
  echo "  info /var/run/charon.pid exists: a charon is running, not starting another"
else
  cat > strongswan.conf <<EOF
charon {
  load = random nonce openssl pem pkcs1 pkcs8 x509 pubkey sha2 hmac kdf kernel-pfkey kernel-pfroute socket-default vici
  port = 0
  port_nat_t = 0
  install_routes = no
  plugins {
    vici {
      socket = unix://$T/charon.vici
    }
  }
  filelog {
    log {
      path = $T/charon.log
      default = 1
      dmn = 2
      lib = 2
      cfg = 2
      flush_line = yes
    }
  }
}
EOF
  cat > swanctl.conf <<'EOF'
connections {
  qnxtest {
    version = 2
    local_addrs = 127.0.0.1
    remote_addrs = 127.0.0.2
    proposals = aes128-sha256-modp2048
    local {
      auth = psk
      id = moon.qnx.test
    }
    remote {
      auth = psk
      id = sun.qnx.test
    }
    children {
      net {
        local_ts = 10.1.0.0/16
        remote_ts = 10.2.0.0/16
        esp_proposals = aes128-sha256
      }
    }
  }
}
secrets {
  ike-qnx {
    id-1 = moon.qnx.test
    id-2 = sun.qnx.test
    secret = "qnx-test-secret"
  }
}
EOF
  STRONGSWAN_CONF=$T/strongswan.conf; export STRONGSWAN_CONF
  "$CHARON" > charon.out 2>&1 &
  CPID=$!; MYPID=1
  U="--uri unix://$T/charon.vici"
  # Ready = swanctl gets an answer over vici (the socket appears while the
  # plugins load, before charon has finished starting).
  i=0; up=0
  while [ $i -lt 20 ] && kill -0 $CPID 2>/dev/null; do
    if [ -S "$T/charon.vici" ] && swanctl --stats $U > stats.out 2>&1; then up=1; break; fi
    sleep 1; i=$((i+1))
  done
  if [ $up -eq 1 ]; then
    ok "charon started (pid $CPID), answering over vici after ${i}s"
    out=$(swanctl --version $U 2>&1)
    [ "$out" = "strongSwan swanctl 6.0.6" ] && ok "swanctl --version: $out" || bad "swanctl --version: $out"
    out=$(swanctl --version --daemon $U 2>&1)
    case "$out" in "strongSwan 6.0.6 charon ("*) ok "swanctl --version --daemon: $out" ;; *) bad "daemon version: $out" ;; esac
    out=$(cat stats.out)
    case "$out" in *"uptime:"*"worker threads:"*) ok "swanctl --stats: uptime and worker threads" ;; *) bad "swanctl --stats: $out" ;; esac
    out=$(swanctl --load-all --file "$T/swanctl.conf" $U 2>&1)
    echo "$out" | sed -n '/loaded\|failed/p' | sed 's/^/  info /'
    echo "$out" | grep -q "loaded ike secret 'ike-qnx'" && ok "load-all: PSK secret loaded" || bad "load-all secret: $out"
    echo "$out" | grep -q "successfully loaded 1 connections, 0 unloaded" && ok "load-all: 1 connection loaded" || bad "load-all conns: $out"
    out=$(swanctl --list-conns $U 2>&1)
    echo "$out" | grep -q "^qnxtest: IKEv2" && echo "$out" | grep -q "remote: 10.2.0.0/16" \
      && echo "$out" | grep -q "id: sun.qnx.test" \
      && ok "list-conns: qnxtest IKEv2, child remote 10.2.0.0/16, remote id sun.qnx.test" || bad "list-conns: $out"
    out=$(swanctl --list-sas $U 2>&1); rc=$?
    [ $rc -eq 0 ] && ok "list-sas: no SAs, rc 0" || bad "list-sas: rc=$rc $out"
    kill -0 $CPID 2>/dev/null && ok "charon still running after the vici checks" || bad "charon exited during the vici checks"
  else
    if kill -0 $CPID 2>/dev/null; then bad "charon (pid $CPID) not answering over vici after ${i}s"
    else
      wait $CPID; st=$?; CPID=
      # libcharon is a critical feature that needs a kernel IPsec backend
      # (here kernel-pfkey: a PF_KEY socket). A network stack without IPsec
      # support has none, and charon then aborts by design.
      if grep -q "unmet dependency: CUSTOM:kernel-ipsec" charon.log 2>/dev/null; then
        echo "  info charon cannot start here: no kernel IPsec interface (PF_KEY) in this network stack"
        grep -i "pfkey\|PF_KEY" charon.log | head -n 3 | sed 's/^/  info | /'
      else bad "charon exited (status $st) before answering over vici"; fi
    fi
    [ -S "$T/charon.vici" ] && echo "  info vici socket exists" || echo "  info no vici socket"
    sed 's/^/  info swanctl | /' stats.out 2>/dev/null | tail -n 3
  fi
  if [ $fail -gt 0 ]; then
    echo "  info --- charon.out"; tail -n 20 charon.out 2>/dev/null | sed 's/^/  info | /'
    echo "  info --- charon.log"; tail -n 40 charon.log 2>/dev/null | sed 's/^/  info | /'
  fi
  unset STRONGSWAN_CONF
fi

cleanup; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS strongswan"; exit 0; }
echo "FAIL strongswan ($fail check(s) failed)"; exit 1
