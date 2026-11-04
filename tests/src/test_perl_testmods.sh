#!/bin/sh
# Perl test-helper modules, each used in a small test script whose TAP output
# is checked: Test::Deep (passing and failing deep comparisons with the
# failure's location), Test::Fatal (exception, lives), Test::Warnings
# (warning captured, had_no_warnings), Test::Needs / Test::Requires (skip all
# for a missing module, run for a present one), Test::RequiresInternet (skips
# under NO_NETWORK_TESTING), Try::Tiny (try/catch/finally, $_ and return
# values), Test::LeakTrace (XS: a reference cycle leaks, a plain block
# does not).
echo "== perl-test-deep, -fatal, -warnings, -needs, -requires, -requiresinternet, -leaktrace, perl-try-tiny =="
P=/usr/local/bin/perl; [ -x "$P" ] || P=perl
T=${TMPDIR:-/tmp}/test_perl_testmods.$$; mkdir -p "$T"; trap 'rm -rf "$T"' EXIT
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
# run <name> <perl code>: TAP output (stdout+stderr) in $T/<name>.out
run() { printf '%s\n' "$2" > "$T/$1.t"; "$P" "$T/$1.t" > "$T/$1.out" 2>&1; echo $? > "$T/$1.rc"; }
has() { grep -qF -- "$2" "$T/$1.out" && ok "$1: $3" || { bad "$1: $3 (no '$2')"; sed 's/^/       | /' "$T/$1.out" | head -20; }; }
rc() { [ "$(cat "$T/$1.rc")" = "$2" ] && ok "$1: exit status $2" || bad "$1: exit status $(cat "$T/$1.rc"), want $2"; }

run deep 'use Test::More tests => 3; use Test::Deep;
cmp_deeply({ a => [1, 2, { b => "x" }], n => 7 }, { a => [1, 2, { b => re(qr/^x$/) }], n => num(7) }, "deep match");
cmp_deeply([3, 1, 2], bag(1, 2, 3), "bag");
cmp_deeply({ a => [1, 2, { b => "y" }] }, { a => [1, 2, { b => "x" }] }, "deep mismatch");'
has deep "ok 1 - deep match" "re() and num() match"
has deep "ok 2 - bag" "bag() ignores order"
has deep "not ok 3 - deep mismatch" "a mismatch fails"
has deep 'Compared $data->{"a"}[2]{"b"}' "the failure names the location"
rc deep 1

run fatal 'use Test::More tests => 3; use Test::Fatal;
like(exception { die "boom\n" }, qr/^boom$/, "exception caught");
is(exception { 1 + 1 }, undef, "no exception");
my $e = exception { die { code => 42 } }; is($e->{code}, 42, "exception object");'
has fatal "ok 1 - exception caught" "exception { die }"
has fatal "ok 2 - no exception" "exception {} is undef when it lives"
has fatal "ok 3 - exception object" "exception returns the thrown ref"
rc fatal 0

run warnings 'use Test::More; use Test::Warnings qw(:all);
like(warning { warn "careful\n" }, qr/careful/, "warning captured");
my @w = warnings { warn "1\n"; warn "2\n" }; is(scalar(@w), 2, "two warnings");
had_no_warnings(); done_testing;'
has warnings "ok 1 - warning captured" "warning {}"
has warnings "ok 2 - two warnings" "warnings {} counts"
has warnings "ok 3 - no (unexpected) warnings" "had_no_warnings"
rc warnings 0
run warnings2 'use Test::More; use Test::Warnings; warn "stray\n"; ok(1, "x"); done_testing;'
has warnings2 "not ok 2 - no (unexpected) warnings" "an unexpected warning fails the test"

run needs 'use Test::More; use Test::Needs "No::Such::Module::QNX"; ok(1); done_testing;'
has needs "1..0 # SKIP" "Test::Needs skips all for a missing module"
has needs "No::Such::Module::QNX" "the skip names the module"
rc needs 0
run needs2 'use Test::More; use Test::Needs "List::Util"; ok(1, "ran"); done_testing;'
has needs2 "ok 1 - ran" "Test::Needs runs when the module is present"
run requires 'use Test::More; use Test::Requires { "No::Such::Module::QNX" => 0 }; ok(1); done_testing;'
has requires "1..0 # SKIP" "Test::Requires skips all for a missing module"
run requires2 'use Test::More; use Test::Requires "Scalar::Util"; ok(defined &Scalar::Util::blessed, "imported"); done_testing;'
has requires2 "ok 1 - imported" "Test::Requires loads a present module"
printf '%s\n' 'use Test::More; use Test::RequiresInternet ("www.example.com" => 80); ok(1); done_testing;' > "$T/net.t"
NO_NETWORK_TESTING=1 "$P" "$T/net.t" > "$T/net.out" 2>&1; echo $? > "$T/net.rc"
has net "1..0 # Skipped: NO_NETWORK_TESTING" "Test::RequiresInternet skips under NO_NETWORK_TESTING"

run trytiny 'use Try::Tiny; my @r;
push @r, try { die "e1\n" } catch { "caught:$_" } finally { push @r, "finally" };
push @r, try { "value" } catch { "no" };
my $err = $@ = "outer"; try { die "inner\n" } catch { push @r, "c:" . (/inner/ ? "inner" : "?") }; push @r, "at:$@";
my @l = try { (1, 2, 3) }; push @r, "list:" . scalar(@l);
try { try { die "deep\n" } catch { die "re:$_" } } catch { chomp(my $x = $_); push @r, $x };
print join("|", @r), "\n";'
has trytiny "finally|caught:e1" "catch gets the error in \$_, finally runs"
has trytiny "|value|" "try returns its value"
has trytiny "|c:inner|at:outer|" "\$@ is left alone"
has trytiny "|list:3|" "list context"
has trytiny "|re:deep" "rethrow from catch"

run leak 'use Test::LeakTrace;
my $cyc = leaked_count { my %h; $h{self} = \%h; };
my $none = leaked_count { my @a = (1 .. 10); my %h = (a => [1, 2]); };
my @info = leaked_info { my $x; $x = \$x; };
print "cycle=", ($cyc > 0 ? "leaks" : "none"), " plain=$none info=", (@info ? "yes" : "no"), "\n";'
has leak "cycle=leaks plain=0 info=yes" "a reference cycle leaks, a plain block does not"

[ $fail -eq 0 ] && { echo "PASS perl-test-modules"; exit 0; }
echo "FAIL perl-test-modules ($fail check(s) failed)"; exit 1
