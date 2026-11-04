#!/bin/sh
# IO::Tty / IO::Pty for perl: a pseudo-terminal pair from IO::Pty->new
# (posix_openpt on QNX), its slave's name and isatty(); the window size set
# on the slave and read back (TIOCSWINSZ/TIOCGWINSZ from
# IO::Tty::Constant); a child that makes the slave its controlling terminal
# (make_slave_controlling_terminal) writes a line and the parent reads it
# from the master with the terminal's output processing applied ("\n" ->
# "\r\n", ONLCR); and the master side writes a line the child reads back
# through the line discipline (echo off, canonical mode).
echo "== perl-io-tty =="
out=$(perl -e '
use strict; use warnings;
use IO::Pty; use IO::Tty qw(TIOCSWINSZ TIOCGWINSZ); use IO::Tty::Constant;
my $fail = 0;
sub ok { my ($c, $w) = @_; print(($c ? "  ok   " : "  FAIL "), "$w\n"); $fail++ unless $c; }
print "  info IO::Tty $IO::Tty::VERSION, perl $^V\n";
ok($IO::Tty::VERSION eq "1.31", "IO::Tty version 1.31");
my $pty = IO::Pty->new; ok(defined $pty, "IO::Pty->new");
my $name = $pty->ttyname; print "  info slave $name\n";
ok(defined $name && $name =~ m{^/dev/}, "ttyname under /dev");
my $slave = $pty->slave; ok(defined $slave && -t $slave, "slave is a tty");
ok(defined IO::Tty::Constant::TIOCSWINSZ(), "TIOCSWINSZ constant defined");
my $ws = pack("S4", 33, 101, 0, 0);
ok(ioctl($slave, TIOCSWINSZ, $ws), "TIOCSWINSZ 33x101 on the slave");
my $got = pack("S4", 0, 0, 0, 0);
ok(ioctl($pty, TIOCGWINSZ, $got) && join("x", (unpack("S4", $got))[0,1]) eq "33x101", "TIOCGWINSZ on the master reads 33x101");

pipe(my $r, my $w) or die;
my $pid = fork; die "fork" unless defined $pid;
if ($pid == 0) {
  close $r; $pty->make_slave_controlling_terminal; my $s = $pty->slave; close $pty;
  require POSIX;
  my $t = POSIX::Termios->new; $t->getattr(fileno $s);
  $t->setoflag($t->getoflag | POSIX::OPOST() | 0x04); # OPOST|ONLCR (0x04 on QNX and Linux)
  $t->setlflag(($t->getlflag | POSIX::ICANON()) & ~POSIX::ECHO());
  $t->setattr(fileno $s, POSIX::TCSANOW());
  syswrite $w, "ready\n";
  syswrite $s, "hello from the child\n";
  my $line = <$s>; $line = "" unless defined $line; chomp $line;
  syswrite $w, "child read: $line\n";
  POSIX::_exit(0);
}
close $w;
my $ready = <$r>;
my $buf = ""; my $rin = ""; vec($rin, fileno($pty), 1) = 1;
for (1..50) { my $rout; last unless select($rout = $rin, undef, undef, 0.2) >= 0;
  if (vec($rout, fileno($pty), 1)) { my $n = sysread $pty, my $chunk, 1024; last unless $n; $buf .= $chunk; last if $buf =~ /\n/; } }
ok($buf eq "hello from the child\r\n", "master reads the child'"'"'s line with ONLCR applied (\\r\\n)");
syswrite $pty, "line from master\n";
my $answer = <$r>; $answer = "" unless defined $answer; chomp $answer;
ok($answer eq "child read: line from master", "child reads the master'"'"'s line on its controlling terminal");
waitpid($pid, 0); ok($? == 0, "child exited 0");
close $slave; $pty->close;
exit($fail ? 1 : 0);
' 2>&1); rc=$?
printf '%s\n' "$out"
[ $rc -eq 0 ] && { echo "PASS perl-io-tty"; exit 0; }
echo "FAIL perl-io-tty (rc=$rc)"; exit 1
