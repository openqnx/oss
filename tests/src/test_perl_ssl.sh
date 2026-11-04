#!/bin/sh
# Net::SSLeay (XS, on this tree's OpenSSL) and IO::Socket::SSL on loopback:
# OpenSSL version, SHA-256 known answer, RAND_bytes, a PEM certificate read
# and its subject; then a TLS server in a child process on 127.0.0.1 with a
# self-signed certificate (test-only key, below; valid 2000-2125 so a target
# clock that is off does not matter) and clients that: verify it against
# that certificate and exchange a line (TLS version, cipher, peer subject);
# fail verification against the Mozilla CA bundle (untrusted issuer); fail on
# a host name mismatch; and fetch over HTTPS with Net::HTTPS (Net::HTTP on
# IO::Socket::SSL).
echo "== perl-net-ssleay, perl-io-socket-ssl =="
P=/usr/local/bin/perl; [ -x "$P" ] || P=perl
T=${TMPDIR:-/tmp}/test_perl_ssl.$$; mkdir -p "$T"; trap 'rm -rf "$T"' EXIT
# A self-signed EC P-256 test certificate for CN=localhost, SAN localhost
# and 127.0.0.1, and its key: made for this test only, secures nothing.
cat > "$T/cert.pem" <<'EOF'
-----BEGIN CERTIFICATE-----
MIIBljCCAT2gAwIBAgIBATAKBggqhkjOPQQDAjAsMRIwEAYDVQQDDAlsb2NhbGhv
c3QxFjAUBgNVBAoMDXFueC1wb3J0LXRlc3QwIBcNMDAwMTAxMDAwMDAwWhgPMjEy
NTAxMDEwMDAwMDBaMCwxEjAQBgNVBAMMCWxvY2FsaG9zdDEWMBQGA1UECgwNcW54
LXBvcnQtdGVzdDBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABPB8bFVfjg7R6q+w
FKeUt05kehme+H/Zkh+syc63AIwVBEWSQRtcE275QEuXOv5HAlo3/HzTmXRx7U5F
EQY/Xl6jTjBMMA8GA1UdEwEB/wQFMAMBAf8wGgYDVR0RBBMwEYIJbG9jYWxob3N0
hwR/AAABMB0GA1UdDgQWBBQ8YnPYCx4Oy6/1WQvMjBVCCyLa7jAKBggqhkjOPQQD
AgNHADBEAiB0PA/OQc/Le4L5hz1g17vMW2OCCgV7uMvtYleL1NAspgIgOMjBXKdR
yGjfFZEujR+TnsSD2eT2kl6F266RrNztBlI=
-----END CERTIFICATE-----
EOF
cat > "$T/key.pem" <<'EOF'
-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgTd8RhGLmxZ+tnMQ3
TpUg/Sgxm3VWyANV1PcKtJAMPJihRANCAATwfGxVX44O0eqvsBSnlLdOZHoZnvh/
2ZIfrMnOtwCMFQRFkkEbXBNu+UBLlzr+RwJaN/x805l0ce1ORREGP15e
-----END PRIVATE KEY-----
EOF
cat > "$T/t.pl" <<'EOF'
use strict; use warnings;
my $fail = 0;
sub ok { my ($c, $w) = @_; print(($c ? "  ok   " : "  FAIL "), "$w\n"); $fail++ unless $c; }
sub is { my ($g, $e, $w) = @_; my $c = defined $g && $g eq $e; ok($c, $w); print "       got '", ($g // 'undef'), "', want '$e'\n" unless $c; }
my $T = shift;

# ---- Net::SSLeay ----
use Net::SSLeay;
my $v = Net::SSLeay::SSLeay_version(0);
print "  info Net::SSLeay $Net::SSLeay::VERSION, $v\n";
ok($v =~ /^OpenSSL 3\./ && Net::SSLeay::OPENSSL_VERSION_NUMBER() >= 0x30000000, "linked OpenSSL 3");
my $md = Net::SSLeay::EVP_get_digestbyname("sha256");
my $ctx = Net::SSLeay::EVP_MD_CTX_create();
Net::SSLeay::EVP_DigestInit($ctx, $md); Net::SSLeay::EVP_DigestUpdate($ctx, "abc");
is(unpack("H*", Net::SSLeay::EVP_DigestFinal($ctx)), "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "SHA-256('abc') through EVP");
Net::SSLeay::EVP_MD_CTX_destroy($ctx);
my $rnd = ""; Net::SSLeay::RAND_bytes($rnd, 32);
ok(length($rnd) == 32 && $rnd ne "\0" x 32, "RAND_bytes");
my $bio = Net::SSLeay::BIO_new_file("$T/cert.pem", "r");
my $x509 = Net::SSLeay::PEM_read_bio_X509($bio); Net::SSLeay::BIO_free($bio);
is(Net::SSLeay::X509_NAME_oneline(Net::SSLeay::X509_get_subject_name($x509)), "/CN=localhost/O=qnx-port-test", "PEM certificate subject");
is(Net::SSLeay::X509_get_fingerprint($x509, "sha256") =~ tr/://dr =~ /^[0-9A-F]{64}$/ ? "fp" : "nofp", "fp", "certificate SHA-256 fingerprint");

# ---- IO::Socket::SSL ----
use IO::Socket::SSL; use Net::HTTPS;
print "  info IO::Socket::SSL $IO::Socket::SSL::VERSION\n";
my $srv = IO::Socket::SSL->new(LocalAddr => "127.0.0.1", LocalPort => 0, Listen => 5, ReuseAddr => 1,
  SSL_server => 1, SSL_cert_file => "$T/cert.pem", SSL_key_file => "$T/key.pem") or die "server: $SSL_ERROR $!";
my $port = $srv->sockport;
my $pid = fork; die "fork: $!" unless defined $pid;
if (!$pid) {
  $SIG{ALRM} = sub { exit 2 }; alarm 60;
  while (1) {
    my $c = $srv->accept or next;
    my $l = <$c>; $l = "" unless defined $l;
    if ($l =~ /^QUIT/) { close $c; last }
    if ($l =~ m{^GET (\S+) HTTP/1\.1}) { my $p = $1; while (my $h = <$c>) { last if $h =~ /^\r?\n$/ }
      my $b = "secure $p"; print $c "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " . length($b) . "\r\nConnection: close\r\n\r\n$b"; }
    else { chomp $l; print $c "echo:$l\n" }
    close $c;
  }
  exit 0;
}
close $srv;
my %verify = (SSL_verify_mode => SSL_VERIFY_PEER, SSL_ca_file => "$T/cert.pem", SSL_verifycn_scheme => "http", SSL_verifycn_name => "localhost");
my $cl = IO::Socket::SSL->new(PeerAddr => "127.0.0.1", PeerPort => $port, %verify);
ok($cl, "client verifies the server against its certificate") or print "       $SSL_ERROR\n";
if ($cl) {
  print $cl "hello tls\n"; my $r = <$cl>;
  is($r, "echo:hello tls\n", "line exchanged over TLS");
  print "  info ", $cl->get_sslversion, " ", $cl->get_cipher, "\n";
  ok($cl->get_sslversion =~ /^TLSv1_[23]$/, "TLS 1.2 or 1.3 negotiated");
  is($cl->peer_certificate("subject"), "/CN=localhost/O=qnx-port-test", "peer certificate subject");
  close $cl;
}
my $bad = IO::Socket::SSL->new(PeerAddr => "127.0.0.1", PeerPort => $port, SSL_verify_mode => SSL_VERIFY_PEER,
  SSL_ca_file => do { require Mozilla::CA; Mozilla::CA::SSL_ca_file() }, SSL_verifycn_name => "localhost", SSL_verifycn_scheme => "http");
ok(!$bad && $SSL_ERROR =~ /certificate verify failed/, "untrusted issuer rejected (Mozilla CA bundle)");
my $mis = IO::Socket::SSL->new(PeerAddr => "127.0.0.1", PeerPort => $port, %verify, SSL_verifycn_name => "not-localhost.example");
ok(!$mis && $SSL_ERROR =~ /hostname verification failed|certificate verify failed/, "host name mismatch rejected");
my $h = Net::HTTPS->new(Host => "127.0.0.1:$port", %verify) or print "       Net::HTTPS: $@ $SSL_ERROR\n";
if ($h) {
  $h->write_request(GET => "/doc");
  my ($code, $mess) = $h->read_response_headers;
  my $body = ""; while (1) { my $buf; my $n = $h->read_entity_body($buf, 1024); last unless $n; $body .= $buf; }
  is("$code $body", "200 secure /doc", "Net::HTTPS GET over IO::Socket::SSL");
} else { ok(0, "Net::HTTPS connect") }
my $q = IO::Socket::SSL->new(PeerAddr => "127.0.0.1", PeerPort => $port, %verify); print $q "QUIT\n" if $q; close $q if $q;
waitpid($pid, 0); is($?, 0, "TLS server child exited 0");
exit($fail ? 1 : 0);
EOF
out=$("$P" "$T/t.pl" "$T" 2>&1); rc=$?
printf '%s\n' "$out"
[ $rc -eq 0 ] && { echo "PASS perl-io-socket-ssl"; exit 0; }
echo "FAIL perl-io-socket-ssl (rc=$rc)"; exit 1
