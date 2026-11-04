#!/bin/sh
# Small XS modules: Clone (deep copies of nested and circular structures,
# blessed objects, independent of the original), B::COW (copy-on-write
# string state and its reference count), and FCGI: a FastCGI responder in a
# child process on a Unix socket, driven by a FastCGI client written here
# (BEGIN_REQUEST, PARAMS, STDIN records); the response's STDOUT records and
# END_REQUEST are checked byte for byte.
echo "== perl-clone, perl-b-cow, perl-fcgi =="
P=/usr/local/bin/perl; [ -x "$P" ] || P=perl
T=${TMPDIR:-/tmp}/test_perl_xs.$$; mkdir -p "$T"; trap 'rm -rf "$T"' EXIT
cat > "$T/t.pl" <<'EOF'
use strict; use warnings;
my $fail = 0;
sub ok { my ($c, $w) = @_; print(($c ? "  ok   " : "  FAIL "), "$w\n"); $fail++ unless $c; }
sub is { my ($g, $e, $w) = @_; my $c = defined $g && $g eq $e; ok($c, $w); print "       got '", ($g // 'undef'), "', want '$e'\n" unless $c; }
my $T = shift;

# ---- Clone ----
use Clone qw(clone);
my $orig = { list => [1, [2, 3]], h => { k => "v" }, obj => bless({ n => 5 }, "QNX::Obj") };
$orig->{self} = $orig;
my $c = clone($orig);
ok($c != $orig && $c->{list} != $orig->{list} && $c->{list}[1] != $orig->{list}[1], "clone makes new containers");
$c->{list}[1][0] = 99; $c->{h}{k} = "changed";
is("$orig->{list}[1][0],$orig->{h}{k}", "2,v", "the original is untouched");
ok($c->{self} == $c, "circular reference points at the clone");
is(ref($c->{obj}) . ":" . $c->{obj}{n}, "QNX::Obj:5", "blessed object cloned with its class");
is(clone("scalar"), "scalar", "clone of a plain scalar");
ok($Clone::VERSION >= 0.47, "Clone version $Clone::VERSION");

# ---- B::COW ----
use B::COW qw(can_cow is_cow cowrefcnt cowrefcnt_max);
ok(can_cow(), "can_cow: this perl has copy-on-write strings");
my $s = "a string long enough to be shared " . time;
my $copy = $s;
ok(is_cow($s) && is_cow($copy), "is_cow after a copy");
my $n = cowrefcnt($s);
my @more = ($s) x 3;
is(cowrefcnt($s), $n + 3, "cowrefcnt counts three more copies");
ok(!is_cow(my $num = 42), "an integer is not a COW string");
ok(cowrefcnt_max() == 255, "cowrefcnt_max 255");

# ---- FCGI ----
use FCGI; use IO::Socket::UNIX; use Socket;
my $path = "$T/fcgi.sock";
my $sock = FCGI::OpenSocket($path, 5) or die "OpenSocket: $!";
my $pid = fork; die "fork: $!" unless defined $pid;
if (!$pid) {
  my %env;
  my $req = FCGI::Request(\*STDIN, \*STDOUT, \*STDERR, \%env, $sock);
  while ($req->Accept() >= 0) {
    my $body = ""; read(STDIN, $body, $env{CONTENT_LENGTH} || 0);
    print "Content-Type: text/plain\r\n\r\nmethod=$env{REQUEST_METHOD} query=$env{QUERY_STRING} body=$body fcgi=", ($req->IsFastCGI ? 1 : 0);
    $req->Finish; last;
  }
  FCGI::CloseSocket($sock); exit 0;
}
sub rec { my ($type, $c) = @_; pack("CCnnCC", 1, $type, 1, length($c), 0, 0) . $c }
sub nv { my %p = @_; join "", map { pack("CC", length($_), length($p{$_})) . $_ . $p{$_} } sort keys %p }
my $cl;
for (1..50) { $cl = IO::Socket::UNIX->new(Peer => $path, Type => SOCK_STREAM) and last; select(undef, undef, undef, 0.1) }
ok($cl, "connected to the FastCGI socket") or die;
my $in = "x=1&y=2";
print $cl rec(1, pack("nCx5", 1, 0))
        . rec(4, nv(REQUEST_METHOD => "POST", QUERY_STRING => "a=b", CONTENT_LENGTH => length $in)) . rec(4, "")
        . rec(5, $in) . rec(5, "");
$cl->flush;
my ($stdout, $end, $buf) = ("", undef, "");
while (1) {
  while (length($buf) >= 8) {
    my ($v, $t, $id, $len, $pad) = unpack("CCnnC", $buf);
    last if length($buf) < 8 + $len + $pad;
    my $content = substr($buf, 8, $len); substr($buf, 0, 8 + $len + $pad) = "";
    if ($t == 6) { $stdout .= $content } elsif ($t == 3) { $end = $content }
  }
  last if defined $end;
  my $n = sysread($cl, my $chunk, 4096); last unless $n; $buf .= $chunk;
}
is($stdout, "Content-Type: text/plain\r\n\r\nmethod=POST query=a=b body=x=1&y=2 fcgi=1", "FastCGI STDOUT from the FCGI responder");
ok(defined $end && unpack("NC", $end) == 0, "END_REQUEST, app status 0");
ok(defined $end && (unpack("NC", $end))[1] == 0, "END_REQUEST, protocol status REQUEST_COMPLETE");
close $cl; waitpid($pid, 0);
is($?, 0, "FCGI child exited 0");
exit($fail ? 1 : 0);
EOF
out=$("$P" "$T/t.pl" "$T" 2>&1); rc=$?
printf '%s\n' "$out"
[ $rc -eq 0 ] && { echo "PASS perl-xs-misc"; exit 0; }
echo "FAIL perl-xs-misc (rc=$rc)"; exit 1
