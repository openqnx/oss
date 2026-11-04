package Devel::CheckLib;
# Cross-build stand-in for Devel::CheckLib (scripts/lib/perl-module.sh,
# PERL_CROSS_CHECKLIB=1). The real module compiles its probe with the build
# machine perl's $Config{cc} and then runs the program: for a QNX target the
# compiler is wrong and the program cannot run here. This version compiles
# and links the same probe with the QNX compiler ($QNX_CHECKLIB_CC) and
# never executes it (as the real module's not_execute option does);
# analyze_binary callbacks still get the binary. Loaded with
# PERL5OPT=-MDevel::CheckLib, before a distribution's bundled inc/ copy.
use strict; use warnings;
use Exporter 'import';
use File::Temp qw(tempdir);
our $VERSION = '1.16';
our @EXPORT = qw(assert_lib check_lib_or_exit check_lib);

sub _list { my $v = shift; return () unless defined $v; ref $v ? @$v : split ' ', $v }
sub _run { my @c = @_; my $q = join ' ', map { my $x = $_; $x =~ s/'/'\\''/g; "'$x'" } @c;
  system("$q >/dev/null 2>&1") == 0 }

sub assert_lib {
  my %a = @_;
  my $cc = $ENV{QNX_CHECKLIB_CC} or die "Devel::CheckLib (cross): QNX_CHECKLIB_CC is not set\n";
  my @cc = split ' ', $cc;
  my @inc = map { "-I$_" } _list($a{incpath}); push @inc, _list($a{INC});
  my @lp = map { "-L$_" } _list($a{libpath});
  my @libs = _list($a{lib});
  for (_list($a{LIBS})) { if (/^-L/) { push @lp, $_ } elsif (/^-l(.+)/) { push @libs, $1 } }
  my @cflags = _list($a{ccflags}); my @ldflags = _list($a{ldflags});
  my @hdr = _list($a{header});
  my $d = tempdir(CLEANUP => 1);
  open my $c, '>', "$d/probe.c" or die "$d/probe.c: $!";
  print $c "#include <$_>\n" for @hdr;
  print $c "int main(int argc, char *argv[]) { (void)argc; (void)argv; ", ($a{function} // 'return 0;'), " }\n";
  close $c;
  my (@missing, @wrong);
  for my $h (@hdr) { _run(@cc, @cflags, @inc, '-c', "$d/probe.c", '-o', "$d/probe.o") or do { push @missing, $h; last } }
  die "Can't link/include C library " . join(', ', map { "'$_'" } @missing) . ", aborting.\n" if @missing;
  @libs = ('') unless @libs;
  for my $l (@libs) {
    if (_run(@cc, @cflags, @inc, "$d/probe.c", '-o', "$d/probe", @lp, ($l eq '' ? () : "-l$l"), @ldflags)) {
      push @wrong, $l if $a{analyze_binary} and !$a{analyze_binary}->($l, "$d/probe");
    } else { push @missing, $l }
    unlink "$d/probe";
  }
  die "Can't link/include C library " . join(', ', map { "'$_'" } @missing) . ", aborting.\n" if @missing;
  die "wrong analysis: " . join(', ', map { "'$_'" } @wrong) if @wrong;
  1;
}
sub check_lib { eval { assert_lib(@_) } ? 1 : 0 }
sub check_lib_or_exit { eval { assert_lib(@_) }; if ($@) { warn $@; exit 0 } 1 }
1;
