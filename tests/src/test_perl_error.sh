#!/bin/sh
# perl-error (Error.pm): try/catch/otherwise/finally blocks with exception
# objects -- Error::Simple with its text, value, file and line, a custom
# Error subclass caught by class, an unmatched error falling through to
# otherwise, rethrow and finally ordering, and $@ compatibility with die.
echo "== perl-error =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=/usr/local/bin/perl; [ -x "$P" ] || P=perl
v=$("$P" -MError -e 'print $Error::VERSION' 2>&1)
case "$v" in 0.*) ok "perl -MError loads (Error $v)" ;; *) bad "perl -MError: $v"; echo "  info @INC: $("$P" -e 'print "@INC"')" ;; esac
out=$("$P" -MError=:try -e '
package QNX::Err; use base qw(Error);
sub new { my ($c, %a) = @_; local $Error::Depth = $Error::Depth + 1; $c->SUPER::new(-text => "qnx failed", %a) }
package main;
my @r;
try { throw Error::Simple("boom", 42) }
catch Error::Simple with { my $e = shift; push @r, "simple:" . $e->text . ":" . $e->value . ":" . ($e->line > 0 ? "line" : "noline") };
try { throw QNX::Err(-value => 7) }
catch Error::Simple with { push @r, "wrong" }
catch QNX::Err with { my $e = shift; push @r, "custom:" . $e->text . ":" . $e->value . ":" . ref($e) };
try { throw QNX::Err() } catch Error::Simple with { push @r, "wrong2" } otherwise { push @r, "otherwise:" . ref(shift) };
my @order;
try { try { throw Error::Simple("inner") } catch Error::Simple with { push @order, "catch"; shift->throw } finally { push @order, "finally" } }
catch Error::Simple with { push @order, "outer:" . shift->text };
push @r, join(">", @order);
try { die "plain die\n" } catch Error::Simple with { (my $t = shift->text) =~ s/\n//; push @r, "die:$t" };
my $v = try { 5 * 8 } catch Error with { -1 };
push @r, "value:$v";
eval { throw Error::Simple("evalled") }; push @r, "eval:" . (ref($@) && $@->isa("Error") ? $@->text : "none");
print join("|", @r), "\n";
' 2>&1)
echo "$out" | sed 's/^/  info /'
for want in "simple:boom:42:line" "custom:qnx failed:7:QNX::Err" "otherwise:QNX::Err" "catch>finally>outer:inner" "die:plain die" "value:40" "eval:evalled"; do
  case "|$out|" in *"|$want|"*) ok "$want" ;; *) bad "$want" ;; esac
done
[ $fail -eq 0 ] && { echo "PASS perl-error"; exit 0; }
echo "FAIL perl-error ($fail check(s) failed)"; exit 1
