# IO::Tty and IO::Pty (pseudo-terminals for Perl; what Expect and
# IPC::Run's pty support are built on), an XS module, built for the target
# perl of this tree. patches/0001: the terminal ioctls (TIOCSWINSZ,
# TIOCSCTTY ...) are probed in <sys/ioctl.h>, where QNX declares them.
VERSION=1.31
TARBALL=https://cpan.metacpan.org/authors/id/T/TO/TODDR/IO-Tty-1.31.tar.gz
SHA256=d597af221628571cbecf35b44520148c44798dfc8a9867774e60453f79d25ff7
DEPS="perl"
BUILDSYS=custom
# Cross build: Makefile.PL only compiles and links its probes (openpty,
# ptsname, posix_openpt, the termios constants...), never runs them, so the
# build machine's perl runs it with the QNX compiler named on its command
# line; it writes xssubs.c, IO/Tty/Constant.pm and the -DHAVE_... list. The
# generated Makefile is then driven with the target perl's settings, read
# from the staged Config_heavy.pl: its CORE headers, ccflags, -fPIC and
# -shared, and its site arch directory (site_perl/<version>/<archname>, on
# the target's @INC). xsubpp is the build machine's ExtUtils::ParseXS; its
# output is plain C compiled against the target perl's headers.
# PERL_HDRS is emptied: it lists the build machine perl's headers
# (patchlevel-debian.h ...) as dependencies of Tty.o.
# Makefile.PL also looks for clone devices (/dev/ptmx ...) on the machine it
# runs on; that answer is the build machine's, so -DHAVE_DEV_PTMX is taken
# out again. IO::Tty opens ptys with posix_openpt(), which QNX has and which
# the module tries first.
_pcfg() { sed -n "s/^$1='\(.*\)'\$/\1/p" "$(ls "$STAGE_SYSROOT$PREFIX"/lib/perl5/*/*/Config_heavy.pl | head -n 1)"; }
_pmk() {
  local core; core=$(dirname "$(ls "$STAGE_SYSROOT$PREFIX"/lib/perl5/*/*/Config_heavy.pl | head -n 1)")/CORE
  make -C "$B" CC="qcc -V$QCC_TARGET" LD="qcc -V$QCC_TARGET" CCFLAGS="$(_pcfg ccflags)" OPTIMIZE="-O2" \
    CCCDLFLAGS="$(_pcfg cccdlflags)" LDDLFLAGS="$(_pcfg lddlflags)" DLEXT=so LDFLAGS= \
    PERL_INC="$core" PERL_ARCHLIBDEP="$core/.." PERL_INCDEP="$core" PERL_HDRS= \
    INSTALLSITEARCH="$(_pcfg installsitearch)" INSTALLSITELIB="$(_pcfg installsitearch)" \
    INSTALLSITEMAN3DIR="$PREFIX/share/man/man3" "$@"
}
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && \
  PERL_MM_USE_DEFAULT=1 perl Makefile.PL CC="qcc -V$QCC_TARGET" CCFLAGS="$(_pcfg ccflags)" OPTIMIZE="-O2" LDFLAGS="" \
    INSTALLDIRS=site && \
  sed -i 's/-DHAVE_DEV_PTMX //g' Makefile
}
do_build() { _pmk; }
# perllocal.pod (every module install appends to it: a file two packages
# would share) and .packlist (it lists the DESTDIR paths) are not shipped.
do_install() {
  _pmk DESTDIR="$STAGE" install && \
  find "$STAGE$PREFIX/lib/perl5" \( -name perllocal.pod -o -name .packlist \) -delete
}
