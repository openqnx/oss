#!/bin/sh
# The libwww-perl stack: URI (normalisation, relative resolution, escaping,
# query forms, otpauth with MIME::Base32), HTTP::Date, HTTP::Message
# (headers, parse, gzip round trip, clone via Clone), LWP::MediaTypes,
# HTTP::Negotiate, HTTP::Cookies, HTTP::CookieJar, File::Listing,
# WWW::RobotRules, Protocol::WebSocket (RFC 6455 handshake and frames),
# then an HTTP::Daemon on 127.0.0.1 in a child process serving
# LWP::UserAgent (GET, POST echo, 404, redirect, cookies), Net::HTTP (raw
# HTTP/1.1 request) and the lwp-request script.
echo "== perl-libwww and the HTTP modules =="
P=/usr/local/bin/perl; [ -x "$P" ] || P=perl
T=${TMPDIR:-/tmp}/test_perl_http.$$; mkdir -p "$T"; trap 'rm -rf "$T"' EXIT
LWPREQ=/usr/local/bin/lwp-request; [ -x "$LWPREQ" ] || LWPREQ=lwp-request
cat > "$T/t.pl" <<'EOF'
use strict; use warnings;
my $fail = 0;
sub ok { my ($c, $w) = @_; print(($c ? "  ok   " : "  FAIL "), "$w\n"); $fail++ unless $c; }
sub is { my ($g, $e, $w) = @_; my $c = defined $g && $g eq $e; ok($c, $w); print "       got '", ($g // 'undef'), "', want '$e'\n" unless $c; }

# ---- URI, MIME::Base32 ----
use URI; use URI::Escape; use MIME::Base32;
is(URI->new("HTTP://WWW.Example.COM:80/%7efoo/%2a?q=1#Frag")->canonical->as_string,
   "http://www.example.com/~foo/*?q=1#Frag", "URI canonical: scheme/host case, default port, unreserved unescaped");
is(URI->new_abs("../d/e?q", "http://h/a/b/c")->as_string, "http://h/a/d/e?q", "URI new_abs resolves ../");
is(URI->new("http://h/a/d/e")->rel("http://h/a/b/c")->as_string, "../d/e", "URI rel");
is(uri_escape("a b&c/\x{e9}"), "a%20b%26c%2F%E9", "uri_escape");
is(uri_unescape("%41%20b"), "A b", "uri_unescape");
my $u = URI->new("http://h/s"); $u->query_form(b => 2, a => "x y");
is($u->as_string, "http://h/s?b=2&a=x+y", "URI query_form");
is(URI->new("mailto:joe\@example.com")->to, "joe\@example.com", "URI mailto");
is(MIME::Base32::encode("foobar"), "MZXW6YTBOI", "MIME::Base32 encode foobar (RFC 4648)");
is(MIME::Base32::decode("MZXW6YTBOI"), "foobar", "MIME::Base32 decode");
my $otp = URI->new("otpauth://totp/Example:alice\@example.com?secret=JBSWY3DPEHPK3PXP&issuer=Example");
is($otp->secret, "Hello!\xde\xad\xbe\xef", "URI::otpauth secret (Base32-decoded)");

# ---- HTTP::Date ----
use HTTP::Date qw(time2str str2time time2iso);
is(time2str(784111777), "Sun, 06 Nov 1994 08:49:37 GMT", "HTTP::Date time2str");
is(str2time("Sunday, 06-Nov-94 08:49:37 GMT"), 784111777, "HTTP::Date str2time RFC 850");
is(str2time("1994-11-06T08:49:37Z"), 784111777, "HTTP::Date str2time ISO 8601");
is(str2time("Sun Nov  6 08:49:37 1994", "GMT"), 784111777, "HTTP::Date str2time asctime");

# ---- HTTP::Message ----
use HTTP::Request; use HTTP::Response; use HTTP::Headers;
my $req = HTTP::Request->new(GET => "http://example.com/x", [Accept => "text/html", "X-A" => 1]);
is($req->as_string("\n"), "GET http://example.com/x\nAccept: text/html\nX-A: 1\n\n", "HTTP::Request as_string");
my $res = HTTP::Response->parse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain; charset=UTF-8\r\nX-Y: z\r\n\r\nh\xc3\xa9");
is($res->code . "|" . $res->message . "|" . $res->header("X-Y"), "404|Not Found|z", "HTTP::Response parse");
is($res->decoded_content, "h\x{e9}", "decoded_content (UTF-8 charset)");
is($res->content_charset, "UTF-8", "content_charset");
my $m = HTTP::Response->new(200, "OK", ["Content-Type" => "text/plain"], "compress me " x 20);
$m->encode("gzip");
is($m->header("Content-Encoding"), "gzip", "encode gzip sets Content-Encoding");
ok(length($m->content) < 240, "gzip content is smaller (" . length($m->content) . " bytes)");
is($m->decoded_content, "compress me " x 20, "gzip round trip");
my $c = $req->clone; $c->header(Accept => "text/plain");
is($req->header("Accept") . "|" . $c->header("Accept"), "text/html|text/plain", "HTTP::Request clone is independent");
my $h = HTTP::Headers->new; $h->push_header("Set-Cookie" => "a=1"); $h->push_header("Set-Cookie" => "b=2");
is(join(",", $h->header("Set-Cookie")), "a=1,b=2", "HTTP::Headers multi-valued header");

# ---- LWP::MediaTypes, HTTP::Negotiate ----
use LWP::MediaTypes qw(guess_media_type media_suffix);
is(scalar guess_media_type("index.html"), "text/html", "guess_media_type index.html");
is(join(",", guess_media_type("a.tar.gz")), "application/x-tar,gzip", "guess_media_type a.tar.gz (type, encoding)");
ok(grep({ $_ eq "png" } media_suffix("image/png")), "media_suffix image/png has png");
use HTTP::Negotiate qw(choose);
my $vars = [["v-html", 1.0, "text/html", undef, undef, "en", 1000], ["v-plain", 0.9, "text/plain", undef, undef, "en", 500]];
is(scalar choose($vars, HTTP::Headers->new(Accept => "text/plain, text/html;q=0.1")), "v-plain", "HTTP::Negotiate picks by Accept q");
is(scalar choose($vars, HTTP::Headers->new(Accept => "text/*")), "v-html", "HTTP::Negotiate picks by source quality");

# ---- cookies ----
use HTTP::Cookies;
my $jar = HTTP::Cookies->new;
my $r2 = HTTP::Response->new(200, "OK", ["Set-Cookie" => "sid=abc; path=/; domain=.example.com"]);
$r2->request(HTTP::Request->new(GET => "http://www.example.com/"));
$jar->extract_cookies($r2);
my $q2 = HTTP::Request->new(GET => "http://www.example.com/page"); $jar->add_cookie_header($q2);
is($q2->header("Cookie"), "sid=abc", "HTTP::Cookies extract and send back");
my $q3 = HTTP::Request->new(GET => "http://other.org/"); $jar->add_cookie_header($q3);
ok(!defined $q3->header("Cookie"), "HTTP::Cookies not sent to another domain");
use HTTP::CookieJar;
my $cj = HTTP::CookieJar->new;
$cj->add("http://www.example.com/", "lang=en-US; Path=/; Domain=example.com");
$cj->add("http://www.example.com/", "old=1; Path=/; Expires=Thu, 01 Jan 1970 00:00:01 GMT");
is($cj->cookie_header("http://foo.example.com/x"), "lang=en-US", "HTTP::CookieJar domain match, expired dropped");

# ---- File::Listing, WWW::RobotRules ----
use File::Listing qw(parse_dir);
my @l = parse_dir("total 2\ndrwxr-xr-x  2 root root 4096 Jan  1  2020 dir\n-rw-r--r--  1 u g 123 Mar  4  2021 file.txt\n", "GMT");
is(scalar(@l), 2, "File::Listing parses two entries");
is(join(",", @{$l[1]}[0,1,2]), "file.txt,f,123", "File::Listing name,type,size");
is($l[0][1], "d", "File::Listing directory type");
is($l[1][3], 1614816000, "File::Listing mtime Mar 4 2021 GMT");
use WWW::RobotRules;
my $rr = WWW::RobotRules->new("TestBot/1.0");
$rr->parse("http://example.com/robots.txt", "User-agent: *\nDisallow: /private\n\nUser-agent: TestBot\nDisallow: /nobots\n");
ok(!$rr->allowed("http://example.com/nobots/x"), "RobotRules: /nobots disallowed for TestBot");
ok($rr->allowed("http://example.com/private/x"), "RobotRules: TestBot record overrides *");
ok($rr->allowed("http://example.com/ok"), "RobotRules: /ok allowed");

# ---- Protocol::WebSocket ----
use Protocol::WebSocket::Handshake::Server; use Protocol::WebSocket::Frame;
my $hs = Protocol::WebSocket::Handshake::Server->new;
$hs->parse("GET /chat HTTP/1.1\r\nHost: server.example.com\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
         . "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nOrigin: http://example.com\r\nSec-WebSocket-Version: 13\r\n\r\n");
ok($hs->is_done, "WebSocket server handshake parsed");
ok($hs->to_string =~ /Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK\+xOo=\r\n/, "Sec-WebSocket-Accept (RFC 6455 example)");
my $bytes = Protocol::WebSocket::Frame->new("hello")->to_bytes;
is(unpack("H*", $bytes), "810568656c6c6f", "WebSocket text frame bytes");
my $f = Protocol::WebSocket::Frame->new; $f->append($bytes);
is($f->next, "hello", "WebSocket frame parsed back");

# ---- HTTP::Daemon + LWP::UserAgent + Net::HTTP on loopback ----
use HTTP::Daemon; use HTTP::Status qw(:constants); use LWP::UserAgent; use Net::HTTP;
my $d = HTTP::Daemon->new(LocalAddr => "127.0.0.1", LocalPort => 0, ReuseAddr => 1) or die "HTTP::Daemon: $!";
my $base = $d->url; print "  info daemon at $base\n";
ok($base =~ m{^http://127\.0\.0\.1:\d+/$}, "HTTP::Daemon url");
my $pid = fork; die "fork: $!" unless defined $pid;
if (!$pid) {
  SERVE: while (my $conn = $d->accept) {
    while (my $r = $conn->get_request) {
      my $p = $r->uri->path;
      if ($p eq "/hello") { $conn->send_response(HTTP::Response->new(200, "OK", ["Content-Type" => "text/plain"], "hello world")) }
      elsif ($p eq "/echo") { $conn->send_response(HTTP::Response->new(200, "OK", ["Content-Type" => "text/plain"], uc($r->content) . "|" . $r->method)) }
      elsif ($p eq "/redir") { $conn->send_redirect("/hello") }
      elsif ($p eq "/setcookie") { $conn->send_response(HTTP::Response->new(200, "OK", ["Set-Cookie" => "k=v42; path=/"], "set")) }
      elsif ($p eq "/getcookie") { $conn->send_response(HTTP::Response->new(200, "OK", [], $r->header("Cookie") // "none")) }
      elsif ($p eq "/quit") { $conn->send_response(HTTP::Response->new(200, "OK", [], "bye")); $conn->close; last SERVE }
      else { $conn->send_error(HTTP_NOT_FOUND) }
    }
    $conn->close;
  }
  exit 0;
}
my $ua = LWP::UserAgent->new(timeout => 20, cookie_jar => HTTP::Cookies->new);
my $g = $ua->get("${base}hello");
is($g->code . " " . $g->decoded_content, "200 hello world", "LWP GET from HTTP::Daemon");
is($g->header("Content-Type"), "text/plain", "LWP response Content-Type");
my $po = $ua->post("${base}echo", Content => "posted data");
is($po->decoded_content, "POSTED DATA|POST", "LWP POST echoed by the daemon");
is($ua->get("${base}missing")->code, 404, "LWP 404 for an unknown path");
my $rd = $ua->get("${base}redir");
is($rd->code . " " . $rd->decoded_content . " " . ($rd->previous ? $rd->previous->code : "-"), "200 hello world 301", "LWP follows a redirect");
$ua->get("${base}setcookie");
is($ua->get("${base}getcookie")->decoded_content, "k=v42", "LWP cookie jar round trip");
my $nh = Net::HTTP->new(Host => "127.0.0.1:" . URI->new($base)->port) or die "Net::HTTP: $@";
$nh->write_request(GET => "/hello", "User-Agent" => "qnx-test");
my ($code, $mess, %hh) = $nh->read_response_headers;
my $body = ""; while (1) { my $buf; my $n = $nh->read_entity_body($buf, 1024); die "read: $!" unless defined $n; last unless $n; $body .= $buf; }
is("$code $mess $body", "200 OK hello world", "Net::HTTP raw request");
print "BASE $base\n";
$ENV{QNX_TEST_LWPREQ} and do {
  my $out = `$ENV{QNX_TEST_LWPREQ} ${base}hello 2>&1`;
  is($out, "hello world", "lwp-request script fetches from the daemon");
};
$ua->get("${base}quit"); waitpid($pid, 0);
is($?, 0, "daemon child exited 0");
exit($fail ? 1 : 0);
EOF
out=$(QNX_TEST_LWPREQ="$LWPREQ" "$P" "$T/t.pl" 2>&1); rc=$?
printf '%s\n' "$out" | grep -v '^BASE '
[ $rc -eq 0 ] && { echo "PASS perl-libwww"; exit 0; }
echo "FAIL perl-libwww (rc=$rc)"; exit 1
