#!/bin/sh
# XML::Parser (XS, on expat): Tree style result, handler events with
# attributes and character data, namespaces, a well-formedness error with its
# line number, a document in ISO-8859-2 decoded through the encoding map that
# File::ShareDir finds in XML::Parser's share directory; File::ShareDir and
# Class::Inspector directly; XML::Simple XMLin/XMLout round trip (on
# XML::Parser) with known structures.
echo "== perl-xml-parser, perl-xml-simple, perl-file-sharedir, perl-class-inspector =="
P=/usr/local/bin/perl; [ -x "$P" ] || P=perl
T=${TMPDIR:-/tmp}/test_perl_xml.$$; mkdir -p "$T"; trap 'rm -rf "$T"' EXIT
cat > "$T/t.pl" <<'EOF'
use strict; use warnings;
my $fail = 0;
sub ok { my ($c, $w) = @_; print(($c ? "  ok   " : "  FAIL "), "$w\n"); $fail++ unless $c; }
sub is { my ($g, $e, $w) = @_; my $c = defined $g && $g eq $e; ok($c, $w); print "       got '", ($g // 'undef'), "', want '$e'\n" unless $c; }
my $T = shift;

use XML::Parser;
print "  info XML::Parser $XML::Parser::VERSION, expat ", XML::Parser::Expat::expat_version(), "\n";
my $tree = XML::Parser->new(Style => "Tree")->parse('<r a="1"><x>hi</x><y/></r>');
is(join(",", $tree->[0], $tree->[1][0]{a}, $tree->[1][1], $tree->[1][2][2], $tree->[1][3]), "r,1,x,hi,y", "XML::Parser Tree style");
my (@ev, $cd);  # expat may split character data: collected per element
sub flush { push @ev, "C:$cd" if defined $cd; undef $cd }
my $xp = XML::Parser->new(Handlers => {
  Start => sub { my ($e, $t, %a) = @_; flush(); push @ev, "S:$t" . join("", map { " $_=$a{$_}" } sort keys %a) },
  End => sub { flush(); push @ev, "E:$_[1]" },
  Char => sub { $cd .= $_[1] } });
$xp->parse(qq{<?xml version="1.0"?><doc id="7" lang="en"><item n="1">caf&#xe9; &amp; &lt;tea&gt;</item><item n="2"/></doc>});
is(join("|", @ev), "S:doc id=7 lang=en|S:item n=1|C:caf\x{e9} & <tea>|E:item|S:item n=2|E:item|E:doc", "XML::Parser handler events, entities");
my @ns;
XML::Parser->new(Namespaces => 1, Handlers => { Start => sub { my ($e, $t) = @_; push @ns, $e->namespace($t) . "#$t" } })
  ->parse('<a:root xmlns:a="urn:qnx:a"><a:leaf/></a:root>');
is(join(" ", @ns), "urn:qnx:a#root urn:qnx:a#leaf", "XML::Parser namespaces");
eval { XML::Parser->new->parse("<a>\n<b>\n</a>") };
ok($@ =~ /mismatched tag at line 3/, "well-formedness error with its line (" . ($@ =~ /^\s*(.*?)\s*$/m)[0] . ")");
open my $o, ">:raw", "$T/l2.xml" or die;
print $o qq{<?xml version="1.0" encoding="ISO-8859-2"?><t>\xb1\xe6\xb3</t>}; close $o;
my $txt = "";
XML::Parser->new(Handlers => { Char => sub { $txt .= $_[1] } })->parsefile("$T/l2.xml");
is($txt, "\x{105}\x{107}\x{142}", "ISO-8859-2 document decoded through share/iso-8859-2.enc");

# ---- File::ShareDir, Class::Inspector ----
use File::ShareDir qw(dist_dir dist_file);
my $sd = eval { dist_dir("XML-Parser") } // "none: $@";
print "  info XML-Parser share dir $sd\n";
ok(-f "$sd/iso-8859-2.enc" && -f "$sd/windows-1252.enc", "File::ShareDir dist_dir(XML-Parser) has the encoding maps");
is(dist_file("XML-Parser", "koi8-r.enc"), "$sd/koi8-r.enc", "File::ShareDir dist_file");
ok(!eval { dist_dir("No-Such-Dist-QNX"); 1 } && $@ =~ /Failed to find share dir/, "dist_dir of an unknown dist dies");
use Class::Inspector;
ok(Class::Inspector->loaded("XML::Parser") && !Class::Inspector->loaded("No::Such::QNX"), "Class::Inspector loaded");
ok(Class::Inspector->installed("File::ShareDir") && !Class::Inspector->installed("No::Such::QNX"), "Class::Inspector installed");
my $fns = Class::Inspector->functions("File::ShareDir");
ok(grep({ $_ eq "dist_dir" } @$fns) && grep({ $_ eq "module_dir" } @$fns), "Class::Inspector functions");
like_path(Class::Inspector->resolved_filename("XML::Parser"), qr{/XML/Parser\.pm$}, "Class::Inspector resolved_filename");
sub like_path { my ($g, $re, $w) = @_; ok(defined $g && $g =~ $re, $w); }

# ---- XML::Simple ----
use XML::Simple qw(XMLin XMLout);
$XML::Simple::PREFERRED_PARSER = "XML::Parser";
my $x = XMLin('<config logdir="/var/log"><server name="a" port="80"/><server name="b" port="8080"><alias>bb</alias><alias>bbb</alias></server></config>');
is($x->{logdir}, "/var/log", "XMLin attribute");
is($x->{server}{a}{port} . "," . $x->{server}{b}{port}, "80,8080", "XMLin folds <server> by name");
is(join(",", @{$x->{server}{b}{alias}}), "bb,bbb", "XMLin repeated elements to an array");
my $data = { name => "qnx", list => [1, 2, 3], nested => { k => "v&<" } };
my $xml = XMLout($data, RootName => "opt", NoAttr => 1, KeyAttr => []);
my $back = XMLin($xml, ForceArray => ["list"], KeyAttr => []);
is(join("|", $back->{name}, join(",", @{$back->{list}}), $back->{nested}{k}), "qnx|1,2,3|v&<", "XMLout/XMLin round trip");
ok($xml =~ m{<k>v&amp;&lt;</k>}, "XMLout escapes & and <");
XMLout({ a => 1 }, OutputFile => "$T/o.xml", RootName => "x");
is(XMLin("$T/o.xml")->{a}, "1", "XMLout to a file, XMLin from it");
exit($fail ? 1 : 0);
EOF
out=$("$P" "$T/t.pl" "$T" 2>&1); rc=$?
printf '%s\n' "$out"
[ $rc -eq 0 ] && { echo "PASS perl-xml-parser"; exit 0; }
echo "FAIL perl-xml-parser (rc=$rc)"; exit 1
