#!/bin/sh
# GnuPG in a scratch GNUPGHOME: versions (gpg, libgcrypt it runs on, the
# agent over Assuan), a SHA-256 of "abc" with the known digest, symmetric
# encryption round trip (and a wrong passphrase refused), an unprotected
# ed25519/cv25519 key generated in batch mode, public-key encrypt/decrypt,
# clear-signed and detached signatures verified (and a tampered file
# rejected), gpgv with the exported key, gpgtar, and gpgsm importing a fixed
# X.509 certificate (subject and serial known). gpg-agent, keyboxd and
# dirmngr are stopped (gpgconf --kill all) on every exit path.
echo "== gnupg =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-gnupg.XXXXXX) || { echo "FAIL gnupg (mktemp)"; exit 1; }
chmod 700 "$T"
GNUPGHOME=$T/home; export GNUPGHOME
mkdir -m 700 "$GNUPGHOME"
stop_daemons() { gpgconf --kill all >/dev/null 2>&1; }
cleanup() { stop_daemons; cd /; rm -rf "$T"; }
trap cleanup EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
G="gpg --batch --no-tty --pinentry-mode loopback"

V=$(gpg --version 2>&1 | head -n 2 | tr '\n' ' ')
echo "  info $V"
case "$V" in "gpg (GnuPG) 2.4.9 libgcrypt 1.11.0"*) ok "gpg 2.4.9 on libgcrypt 1.11.0" ;; *) bad "gpg --version: $V" ;; esac

D=$(printf abc | gpg --with-colons --print-md SHA256 2>&1)
case "$D" in *BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD*) ok "SHA256(\"abc\") = ba7816bf...f20015ad" ;; *) bad "print-md SHA256: $D" ;; esac

printf 'QNX symmetric secret\n' > plain.txt
if $G --passphrase s3cret --symmetric --cipher-algo AES256 -o sym.gpg plain.txt 2> sym.err \
   && $G --passphrase s3cret -d -o sym.out sym.gpg 2>> sym.err && cmp -s plain.txt sym.out; then
  ok "symmetric AES256 encrypt/decrypt round trip"
else bad "symmetric: $(head -c 300 sym.err)"; fi
# The agent caches the passphrase of a symmetric decryption: forget it first.
gpg-connect-agent 'RELOADAGENT' /bye >/dev/null 2>&1
if [ ! -s sym.gpg ]; then bad "wrong passphrase: no encrypted file to try"
elif $G --passphrase wrong -d -o sym.bad sym.gpg 2> bad.err; then bad "wrong passphrase was accepted"
elif grep -q 'Bad session key' bad.err; then ok "wrong passphrase refused: Bad session key"
else bad "wrong passphrase: $(head -c 300 bad.err)"; fi

AV=$(gpg-connect-agent 'GETINFO version' /bye 2>&1 | head -n 1)
[ "$AV" = "D 2.4.9" ] && ok "gpg-agent answers GETINFO version over Assuan: 2.4.9" || bad "agent GETINFO version: $AV"

UID1="QNX Port Test <test@qnx.invalid>"
if $G --passphrase '' --quick-generate-key "$UID1" ed25519 default never 2> gen.err; then
  ok "ed25519 primary key generated (no passphrase)"
else bad "quick-generate-key: $(head -c 300 gen.err)"; fi
FPR=$(gpg --batch --with-colons --list-keys "$UID1" 2>/dev/null | awk -F: '$1=="fpr"{print $10; exit}')
echo "  info fingerprint $FPR"
if [ ${#FPR} -eq 40 ] && $G --passphrase '' --quick-add-key "$FPR" cv25519 encr never 2> sub.err; then
  ok "cv25519 encryption subkey added"
else bad "quick-add-key: $(head -c 300 sub.err)"; fi
L=$(gpg --batch --with-colons --list-keys "$UID1" 2>/dev/null)
echo "$L" | grep -q '^pub:u:255:22:' && ok "key listing: pub ed25519 (algo 22), ultimately trusted" || bad "pub line: $(echo "$L" | grep '^pub')"
echo "$L" | grep -q '^sub:u:255:18:.*:e:' && ok "key listing: sub cv25519 (algo 18) for encryption" || bad "sub line: $(echo "$L" | grep '^sub')"
echo "$L" | grep -q '^uid:u:.*QNX Port Test <test@qnx.invalid>' && ok "key listing: user id" || bad "uid line"

printf 'message for the key\n' > msg.txt
if $G -r "$FPR" --trust-model always -e -o msg.gpg msg.txt 2> enc.err && $G -d -o msg.out msg.gpg 2> dec.err \
   && cmp -s msg.txt msg.out; then
  ok "public-key encrypt to cv25519 subkey / decrypt round trip"
  grep -q 'cv25519\|ECDH' dec.err && ok "decryption reports the ECDH/cv25519 key" || echo "  info $(head -n 2 dec.err | tr '\n' ' ')"
else bad "encrypt/decrypt: $(head -c 300 enc.err) $(head -c 300 dec.err)"; fi

if $G -u "$FPR" --clearsign -o msg.asc msg.txt 2> cs.err && $G --verify msg.asc 2> v.err; then
  grep -q 'Good signature from "QNX Port Test' v.err && ok "clear-signed message verifies: Good signature" || bad "verify text: $(cat v.err)"
else bad "clearsign/verify: $(head -c 300 cs.err) $(head -c 300 v.err)"; fi
$G -u "$FPR" --detach-sign -o msg.sig msg.txt 2> ds.err
if $G --verify msg.sig msg.txt 2> dv.err; then ok "detached signature verifies"; else bad "detached verify: $(head -c 300 dv.err)"; fi
printf 'message for the key!\n' > msg.txt
if $G --verify msg.sig msg.txt 2> tv.err; then bad "tampered file verified"
else grep -q 'BAD signature' tv.err && ok "tampered file: BAD signature" || bad "tampered: $(head -c 300 tv.err)"; fi
printf 'message for the key\n' > msg.txt

gpg --batch --export "$FPR" > pub.gpg 2>/dev/null
if gpgv --keyring "$T/pub.gpg" msg.sig msg.txt 2> gpgv.err; then ok "gpgv verifies with the exported public key"
else bad "gpgv: $(head -c 300 gpgv.err)"; fi

mkdir -p tree/sub; printf one > tree/a; printf two > tree/sub/b
if gpgtar --create --output tree.tar tree 2> tar.err && mkdir out && gpgtar --extract --directory out tree.tar 2>> tar.err \
   && [ "$(cat out/tree/a out/tree/sub/b 2>/dev/null)" = "onetwo" ]; then
  ok "gpgtar create/extract round trip"
else bad "gpgtar: $(head -c 300 tar.err)"; fi

cat > cert.pem <<'PEM'
-----BEGIN CERTIFICATE-----
MIIBuDCCAV2gAwIBAgIEEjSrzTAKBggqhkjOPQQDAjA5MQswCQYDVQQGEwJDQTEW
MBQGA1UECgwNUU5YIFBvcnQgVGVzdDESMBAGA1UEAwwJa3NiYS50ZXN0MB4XDTI2
MTAzMTIzMDUzMloXDTM2MTAyODIzMDUzMlowOTELMAkGA1UEBhMCQ0ExFjAUBgNV
BAoMDVFOWCBQb3J0IFRlc3QxEjAQBgNVBAMMCWtzYmEudGVzdDBZMBMGByqGSM49
AgEGCCqGSM49AwEHA0IABG+iKOZaIIASblK3XG4vgiXmjkCKPI9fYO1VNRaSb153
W5Mr13lZjgzTWplpJELD/xo0EVfQPrg25hoIGFj8RCejUzBRMB0GA1UdDgQWBBSg
+oVKfGR3dlGstb41JG0Nd955AzAfBgNVHSMEGDAWgBSg+oVKfGR3dlGstb41JG0N
d955AzAPBgNVHRMBAf8EBTADAQH/MAoGCCqGSM49BAMCA0kAMEYCIQCKw3fYDjsQ
oY0zF8A/YH3H5luQ4ny4/O149XfZqOnLYAIhAJRQ24oDxaLzK/zyoqN7ueO5lUbo
Pk/tOnRbiLTsme44
-----END CERTIFICATE-----
PEM
if gpgsm --batch --disable-dirmngr --import cert.pem 2> sm.err; then
  SL=$(gpgsm --batch --with-colons --list-keys 2>/dev/null)
  echo "$SL" | grep -q '^crt:.*:1234ABCD:' && ok "gpgsm: certificate imported, serial 1234ABCD" || bad "gpgsm serial: $(echo "$SL" | grep '^crt')"
  echo "$SL" | grep -q '^uid:.*CN=ksba.test,O=QNX Port Test,C=CA' && ok "gpgsm: subject CN=ksba.test,O=QNX Port Test,C=CA" || bad "gpgsm subject: $(echo "$SL" | grep '^uid')"
  echo "$SL" | grep -q '^fpr:.*CE6A77139790DAA30888133C32CB0582E89E232B' && ok "gpgsm: SHA-1 fingerprint CE6A7713...E89E232B" || bad "gpgsm fpr: $(echo "$SL" | grep '^fpr')"
else bad "gpgsm --import: $(head -c 300 sm.err)"; fi

cleanup; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS gnupg"; exit 0; }
echo "FAIL gnupg ($fail check(s) failed)"; exit 1
