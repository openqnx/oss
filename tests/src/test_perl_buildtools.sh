#!/bin/sh
# Perl module build tools, used on the target the way a CPAN install would:
# a small pure-Perl distribution built, tested and installed (into a
# destdir) with Module::Build and with Module::Build::Tiny; a Makefile.PL
# with File::ShareDir::Install writes the share rules into its Makefile;
# ExtUtils::Config (the target perl's Config, overrides), ExtUtils::Helpers
# (shell-like splitting, man page names), ExtUtils::InstallPaths (site and
# install_base destinations), ExtUtils::HasCompiler (agrees with whether
# the compiler perl names is on PATH), CPAN::Requirements::Dynamic
# (conditions evaluated into prerequisites).
echo "== perl-module-build, -build-tiny, perl-extutils-*, perl-cpan-requirements-dynamic, perl-file-sharedir-install =="
P=/usr/local/bin/perl; [ -x "$P" ] || P=perl
T=${TMPDIR:-/tmp}/test_perl_buildtools.$$; mkdir -p "$T"; trap 'rm -rf "$T"' EXIT
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }

cat > "$T/t.pl" <<'EOF'
use strict; use warnings; use Config;
my $fail = 0;
sub ok { my ($c, $w) = @_; print(($c ? "  ok   " : "  FAIL "), "$w\n"); $fail++ unless $c; }
sub is { my ($g, $e, $w) = @_; my $c = defined $g && $g eq $e; ok($c, $w); print "       got '", ($g // 'undef'), "', want '$e'\n" unless $c; }
use ExtUtils::Config;
my $ec = ExtUtils::Config->new;
is($ec->get("archname"), $Config{archname}, "ExtUtils::Config archname ($Config{archname})");
my $ov = ExtUtils::Config->new({ cc => "my-cc" });
is($ov->get("cc") . "|" . ($ov->exists("cc") ? 1 : 0) . "|" . join(",", keys %{ $ov->values_set }), "my-cc|1|cc", "ExtUtils::Config override");
use ExtUtils::Helpers qw(split_like_shell man1_pagename man3_pagename make_executable detildefy);
is(join("|", split_like_shell(q{a "b c" d\ e 'f g'})), "a|b c|d e|f g", "split_like_shell");
is(man1_pagename("script/lwp-x"), "lwp-x.$Config{man1ext}", "man1_pagename");
is(man3_pagename("lib/Foo/Bar.pm"), "Foo::Bar.$Config{man3ext}", "man3_pagename");
my $T = shift; open my $s, ">", "$T/scr" or die; print $s "#!perl\nprint 1\n"; close $s;
make_executable("$T/scr"); ok(-x "$T/scr", "make_executable");
use ExtUtils::InstallPaths;
my $ip = ExtUtils::InstallPaths->new(installdirs => "site", config => $ec);
is($ip->install_destination("lib"), $Config{installsitelib}, "InstallPaths site lib = installsitelib");
is($ip->install_destination("arch"), $Config{installsitearch}, "InstallPaths site arch = installsitearch");
my $ib = ExtUtils::InstallPaths->new(install_base => "/opt/qnx", config => $ec);
is($ib->install_destination("lib"), "/opt/qnx/lib/perl5", "InstallPaths install_base lib");
is($ib->install_destination("bin"), "/opt/qnx/bin", "InstallPaths install_base bin");
use ExtUtils::HasCompiler qw(can_compile_loadable_object);
my $cc = (split " ", $Config{cc})[0];
my $have = $cc =~ m{/} ? -x $cc : grep { -x "$_/$cc" } split /:/, ($ENV{PATH} // "");
my $can = can_compile_loadable_object(quiet => 1) ? 1 : 0;
print "  info perl's cc '$Config{cc}' on PATH: ", ($have ? "yes" : "no"), "; can_compile_loadable_object: $can\n";
is($can, $have ? 1 : 0, "ExtUtils::HasCompiler agrees with the compiler's presence");
use CPAN::Requirements::Dynamic;
my $dyn = CPAN::Requirements::Dynamic->new;
my $pr = $dyn->evaluate({ version => 1, expressions => [
  { condition => [ has_perl => "v5.8.0" ], prereqs => { "Foo::Yes" => "1.2" } },
  { condition => [ has_perl => "v99.0.0" ], prereqs => { "Foo::No" => "1" } },
  { condition => [ is_os => $^O ], prereqs => { "Foo::OS" => "0" } } ] });
my $h = $pr->requirements_for("runtime", "requires")->as_string_hash;
is(join(",", map { "$_=$h->{$_}" } sort keys %$h), "Foo::OS=0,Foo::Yes=1.2", "CPAN::Requirements::Dynamic evaluates conditions ($^O)");
exit($fail ? 1 : 0);
EOF
# ExtUtils::HasCompiler makes its temporary directory in the current
# directory when it is loaded: run from $T (the test may start in /, which
# is read-only on QNX).
out=$(cd "$T" && "$P" "$T/t.pl" "$T" 2>&1) || fail=$((fail+1))
printf '%s\n' "$out"

# a pure-Perl distribution: lib/QNX/Hello.pm, t/hello.t, META.json
mkdist() {
  d=$T/$1; mkdir -p "$d/lib/QNX" "$d/t"
  printf '%s\n' 'package QNX::Hello;' 'our $VERSION = "1.0";' 'sub hi { "hello $_[0]" }' '1;' > "$d/lib/QNX/Hello.pm"
  printf '%s\n' Build.PL META.json lib/QNX/Hello.pm t/hello.t > "$d/MANIFEST"
  printf '%s\n' 'use Test::More tests => 1; use QNX::Hello; is(QNX::Hello::hi("qnx"), "hello qnx", "hi");' > "$d/t/hello.t"
  printf '%s\n' '{"name":"QNX-Hello","version":"1.0","abstract":"x","author":["x"],"license":["perl_5"],"dynamic_config":0,' \
    '"meta-spec":{"version":2},"release_status":"stable","prereqs":{}}' > "$d/META.json"
}
# build <dir> <label>: perl Build.PL, ./Build, ./Build test, ./Build install --destdir
build() {
  ( cd "$T/$1" && "$P" Build.PL > build.log 2>&1 && "$P" Build >> build.log 2>&1 \
    && "$P" Build test > test.log 2>&1 && "$P" Build install --destdir "$T/$1-dest" >> build.log 2>&1 )
  r=$?
  [ $r -eq 0 ] && ok "$2: Build.PL, Build, Build install" || { bad "$2: build (rc=$r)"; sed 's/^/       | /' "$T/$1/build.log" | tail -15; }
  grep -q "^t/hello.t .. ok" "$T/$1/test.log" && grep -q "^Result: PASS" "$T/$1/test.log" && ok "$2: Build test passes" \
    || { bad "$2: Build test"; sed 's/^/       | /' "$T/$1/test.log" | tail -10; }
  f=$(find "$T/$1-dest" -name Hello.pm | head -n 1)
  [ -n "$f" ] && PERL5LIB=$(dirname "$(dirname "$f")") "$P" -MQNX::Hello -e 'exit(QNX::Hello::hi("x") eq "hello x" ? 0 : 1)' \
    && ok "$2: installed module loads from the destdir ($(echo "$f" | sed "s|$T/$1-dest||"))" || bad "$2: installed QNX/Hello.pm"
}
mkdist mb
printf '%s\n' 'use Module::Build; Module::Build->new(module_name => "QNX::Hello", license => "perl", dist_abstract => "x", dist_author => "x")->create_build_script;' > "$T/mb/Build.PL"
build mb "Module::Build $("$P" -MModule::Build -e 'print $Module::Build::VERSION' 2>/dev/null)"
mkdist mbt
printf '%s\n' 'use Module::Build::Tiny; Build_PL();' > "$T/mbt/Build.PL"
build mbt "Module::Build::Tiny $("$P" -MModule::Build::Tiny -e 'print $Module::Build::Tiny::VERSION' 2>/dev/null)"

# File::ShareDir::Install in a Makefile.PL (writing the Makefile needs no make)
mkdist fsdi; mkdir -p "$T/fsdi/share"; echo data > "$T/fsdi/share/file.txt"
printf '%s\n' 'use ExtUtils::MakeMaker; use File::ShareDir::Install; install_share "share";' \
  'WriteMakefile(NAME => "QNX::Hello", VERSION => "1.0");' 'package MY; use File::ShareDir::Install qw(postamble);' > "$T/fsdi/Makefile.PL"
( cd "$T/fsdi" && "$P" Makefile.PL > mf.log 2>&1 )
grep -qF "'share/file.txt' '\$(INST_LIB)/auto/share/dist/\$(DISTNAME)/file.txt'" "$T/fsdi/Makefile" 2>/dev/null \
  && ok "File::ShareDir::Install: the Makefile installs share/file.txt to auto/share/dist/\$(DISTNAME)" \
  || { bad "File::ShareDir::Install postamble"; sed 's/^/       | /' "$T/fsdi/mf.log" | tail -10; }

[ $fail -eq 0 ] && { echo "PASS perl-build-tools"; exit 0; }
echo "FAIL perl-build-tools ($fail check(s) failed)"; exit 1
