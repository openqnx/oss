#!/bin/sh
# HTML::Parser (XS): the event stream of a small document (start/end tags,
# attributes, text, comments, declarations), HTML::Entities both ways,
# HTML::LinkExtor (with URI), HTML::HeadParser (title, meta into
# HTTP::Headers), HTML::TokeParser; HTML::Tagset's element tables;
# IO::HTML's charset sniffing (<meta charset>, BOM) and decoding read;
# Encode::Locale's locale encodings.
echo "== perl-html-parser, perl-html-tagset, perl-io-html, perl-encode-locale =="
P=/usr/local/bin/perl; [ -x "$P" ] || P=perl
T=${TMPDIR:-/tmp}/test_perl_html.$$; mkdir -p "$T"; trap 'rm -rf "$T"' EXIT
cat > "$T/t.pl" <<'EOF'
use strict; use warnings;
my $fail = 0;
sub ok { my ($c, $w) = @_; print(($c ? "  ok   " : "  FAIL "), "$w\n"); $fail++ unless $c; }
sub is { my ($g, $e, $w) = @_; my $c = defined $g && $g eq $e; ok($c, $w); print "       got '", ($g // 'undef'), "', want '$e'\n" unless $c; }
my $T = shift;

# ---- HTML::Parser events ----
use HTML::Parser;
my @ev;
my $p = HTML::Parser->new(api_version => 3,
  start_h => [sub { my ($t, $a) = @_; push @ev, "S:$t" . join("", map { " $_=$a->{$_}" } sort keys %$a) }, "tagname, attr"],
  end_h => [sub { push @ev, "E:$_[0]" }, "tagname"],
  text_h => [sub { push @ev, "T:$_[0]" if $_[0] =~ /\S/ }, "dtext"],
  comment_h => [sub { push @ev, "C:$_[0]" }, "text"],
  declaration_h => [sub { push @ev, "D:$_[0]" }, "text"]);
$p->parse('<!DOCTYPE html><html><body class="x" ID=b1><p>caf&eacute; &amp; t&#233;</p><!-- note --><br/><a href="/r?a=1&amp;b=2">L</a></body></html>');
$p->eof;
is(join("|", @ev), "D:<!DOCTYPE html>|S:html|S:body class=x id=b1|S:p|T:caf\x{e9} & t\x{e9}|E:p|C:<!-- note -->|S:br/|S:a href=/r?a=1&b=2|T:L|E:a|E:body|E:html",
   "HTML::Parser event stream");
ok($HTML::Parser::VERSION >= 3.8, "HTML::Parser version $HTML::Parser::VERSION");

# ---- HTML::Entities ----
use HTML::Entities qw(decode_entities encode_entities);
is(decode_entities("&lt;a&gt; &eacute;&#x41;&#66;&nbsp;&hearts;"), "<a> \x{e9}AB\x{a0}\x{2665}", "decode_entities");
is(encode_entities("<\x{e9}> & \"q\""), "&lt;&eacute;&gt; &amp; &quot;q&quot;", "encode_entities");
is(encode_entities("a<b>c", "<>"), "a&lt;b&gt;c", "encode_entities with a character class");

# ---- HTML::LinkExtor, HTML::HeadParser, HTML::TokeParser ----
use HTML::LinkExtor;
my @links;
my $le = HTML::LinkExtor->new(sub { my ($tag, %a) = @_; push @links, "$tag:" . join(",", map { "$_=$a{$_}" } sort keys %a) }, "http://example.com/dir/");
$le->parse('<a href="x.html">x</a><img src="../i.png"><link rel=stylesheet href="/s.css">'); $le->eof;
is(join(" ", @links), "a:href=http://example.com/dir/x.html img:src=http://example.com/i.png link:href=http://example.com/s.css", "HTML::LinkExtor absolute links");
use HTML::HeadParser;
my $hp = HTML::HeadParser->new;
$hp->parse('<html><head><title>Hello QNX</title><meta http-equiv="Refresh" content="5"><meta name="author" content="me"></head><body>x</body></html>');
is($hp->header("Title"), "Hello QNX", "HTML::HeadParser title");
is($hp->header("Refresh"), "5", "HTML::HeadParser http-equiv");
is($hp->header("X-Meta-Author"), "me", "HTML::HeadParser meta name");
use HTML::TokeParser;
my $tp = HTML::TokeParser->new(\'<h1>One</h1><h1>Two <b>2</b></h1>');
my @h; while ($tp->get_tag("h1")) { push @h, $tp->get_trimmed_text("/h1") }
is(join("|", @h), "One|Two 2", "HTML::TokeParser get_tag/get_trimmed_text");

# ---- HTML::Tagset ----
use HTML::Tagset;
ok($HTML::Tagset::emptyElement{br} && $HTML::Tagset::emptyElement{img} && !$HTML::Tagset::emptyElement{p}, "Tagset emptyElement br, img (not p)");
ok($HTML::Tagset::isPhraseMarkup{b} && !$HTML::Tagset::isPhraseMarkup{div}, "Tagset isPhraseMarkup b (not div)");
is(join(",", @{$HTML::Tagset::linkElements{img}}), "src,lowsrc,longdesc,usemap", "Tagset linkElements img");
ok($HTML::Tagset::isHeadElement{title} && $HTML::Tagset::isBodyElement{table}, "Tagset head/body elements");

# ---- IO::HTML ----
use IO::HTML qw(html_file find_charset_in sniff_encoding);
is(find_charset_in('<html><head><meta charset="windows-1252"></head>'), "cp1252", "find_charset_in meta charset");
is(find_charset_in('<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-2">'), "iso-8859-2", "find_charset_in http-equiv");
open my $o, ">:raw", "$T/l2.html" or die; print $o qq{<html><head><meta charset="iso-8859-2"></head><body>\xb1\xe6</body></html>}; close $o;
my $fh = html_file("$T/l2.html"); my $txt = do { local $/; <$fh> }; close $fh;
ok($txt =~ /<body>\x{105}\x{107}<\/body>/, "html_file decodes ISO-8859-2 by its <meta charset>");
open $o, ">:raw", "$T/bom.html" or die; print $o "\xef\xbb\xbf<p>\xc3\xa9</p>"; close $o;
open my $r, "<:raw", "$T/bom.html" or die; my ($enc, $bom) = sniff_encoding($r, "$T/bom.html"); close $r;
is("$enc|" . ($bom ? 1 : 0), "utf-8-strict|1", "sniff_encoding finds the UTF-8 BOM");

# ---- Encode::Locale ----
use Encode; use Encode::Locale;
print "  info locale encoding $Encode::Locale::ENCODING_LOCALE, fs $Encode::Locale::ENCODING_LOCALE_FS, console_in $Encode::Locale::ENCODING_CONSOLE_IN\n";
ok(defined Encode::find_encoding("locale") && defined Encode::find_encoding("locale_fs") && defined Encode::find_encoding("console_out"), "Encode::Locale aliases locale, locale_fs, console_out");
is(decode("locale", "plain ascii"), "plain ascii", "decode(locale) of ASCII");
{ local $ENV{LC_ALL} = "C.UTF-8"; local $ENV{LANG} = "C.UTF-8";
  my $u = `$^X -MEncode -MEncode::Locale -e 'print Encode::find_encoding("locale")->name'`;
  print "  info locale encoding under C.UTF-8: $u\n"; }
exit($fail ? 1 : 0);
EOF
out=$("$P" "$T/t.pl" "$T" 2>&1); rc=$?
printf '%s\n' "$out"
[ $rc -eq 0 ] && { echo "PASS perl-html-parser"; exit 0; }
echo "FAIL perl-html-parser (rc=$rc)"; exit 1
