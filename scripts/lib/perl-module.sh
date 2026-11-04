# Helpers for recipes of Perl modules (CPAN distributions) installed into the
# target perl's site library. Sourced by ports/perl-*/recipe.sh:
#
#   source "$WS/scripts/lib/perl-module.sh"
#   pure Perl:  do_build() { :; }  do_install() { qnx_perl_pure_install [script...]; }
#   XS:         do_configure() { qnx_perl_xs_configure [Makefile.PL args...]; }
#               do_build() { qnx_perl_xs_make; }  do_install() { qnx_perl_xs_install; }
#
# Where things go: the target perl's own Config (the staged Config_heavy.pl of
# ports/perl). Pure Perl modules into installsitelib (site_perl/<version>;
# the versionless site_perl is not in @INC, hazard 110), XS modules (their .pm
# files too) into installsitearch (site_perl/<version>/<archname>), as
# ports/perl-io-tty does.
#
# XS cross build (as ports/perl-io-tty): the build machine's perl runs
# Makefile.PL with the QNX compiler named on its command line (Makefile.PL
# probes only compile and link); the generated Makefile is then driven with
# the target perl's settings: its CORE headers, ccflags, -fPIC/-shared and its
# site arch directory. xsubpp is the build machine's ExtUtils::ParseXS; its
# output is plain C compiled against the target perl's headers. PERL_HDRS is
# emptied: it lists the build machine perl's headers as dependencies.

qnx_perl_config_heavy() { ls "$STAGE_SYSROOT$PREFIX"/lib/perl5/*/*/Config_heavy.pl | head -n 1; }
qnx_perl_cfg() { sed -n "s/^$1='\(.*\)'\$/\1/p" "$(qnx_perl_config_heavy)"; }

# qnx_perl_pure_install [script...]: lib/ as released (.pm, .pod and the data
# files modules read next to themselves, e.g. LWP/media.types) into the site
# library -- or PERL_LIB_DIR, for a distribution that generates modules at
# build time (blib/lib); each named script (path in the source) into bin/
# with the target perl's #! line.
qnx_perl_pure_install() {
  local d="$STAGE$(qnx_perl_cfg installsitelib)" l="${PERL_LIB_DIR:-$S/lib}" sp f
  sp=$(qnx_perl_cfg startperl)
  [ -n "$sp" ] && [ -d "$l" ] || { echo "no target perl Config or no $l"; return 1; }
  mkdir -p "$d" && (cd "$l" && find . -type f ! -name '*.PL' -print0 | \
    while IFS= read -r -d '' f; do install -Dm644 "$f" "$d/$f" || exit 1; done) || return 1
  for f in "$@"; do
    install -d "$STAGE$PREFIX/bin" && \
    { echo "$sp"; sed '1{/^#!/d}' "$S/$f"; } > "$STAGE$PREFIX/bin/$(basename "$f")" && \
    chmod 755 "$STAGE$PREFIX/bin/$(basename "$f")" || return 1
  done
}

# qnx_perl_xs_configure [Makefile.PL args...]: the source copied to $B, then
# Makefile.PL run there by the build machine's perl. PERL_CROSS_CHECKLIB=1:
# Devel::CheckLib probes (which compile with the build machine's compiler
# and run the program) are replaced by scripts/lib/perl-cross/Devel/CheckLib.pm,
# which compiles and links them with the QNX compiler and runs nothing.
# The link line's LD_RUN_PATH="$(LD_RUN_PATH)" is taken out of the generated
# Makefiles: MakeMaker sets it to the -L directories (the staging tree, a
# build-machine path, as RPATH), and set empty it still left an empty
# DT_RPATH entry in the module.
qnx_perl_xs_configure() {
  local opt=${PERL5OPT-}
  [ "${PERL_CROSS_CHECKLIB:-0}" = 1 ] && opt="-I$WS/scripts/lib/perl-cross -MDevel::CheckLib $opt"
  cp -a "$S/." "$B/" && cd "$B" && \
  PERL5OPT="$opt" QNX_CHECKLIB_CC="qcc -V$QCC_TARGET" PERL_MM_USE_DEFAULT=1 perl Makefile.PL CC="qcc -V$QCC_TARGET" LD="qcc -V$QCC_TARGET" \
    CCFLAGS="$(qnx_perl_cfg ccflags)" OPTIMIZE=-O2 INSTALLDIRS=site "$@" && \
  find "$B" -name Makefile -exec sed -i 's/LD_RUN_PATH="$(LD_RUN_PATH)" //' {} +
}

# qnx_perl_xs_make [make args...]
qnx_perl_xs_make() {
  local core; core=$(dirname "$(qnx_perl_config_heavy)")/CORE
  make -C "$B" CC="qcc -V$QCC_TARGET" LD="qcc -V$QCC_TARGET" CCFLAGS="$(qnx_perl_cfg ccflags)" OPTIMIZE=-O2 \
    CCCDLFLAGS="$(qnx_perl_cfg cccdlflags)" LDDLFLAGS="$(qnx_perl_cfg lddlflags)" DLEXT=so LDFLAGS= \
    PERL_INC="$core" PERL_ARCHLIBDEP="$core/.." PERL_INCDEP="$core" PERL_HDRS= \
    INSTALLSITEARCH="$(qnx_perl_cfg installsitearch)" INSTALLSITELIB="$(qnx_perl_cfg installsitearch)" \
    INSTALLSITEBIN="$PREFIX/bin" INSTALLSITESCRIPT="$PREFIX/bin" \
    INSTALLSITEMAN1DIR="$PREFIX/share/man/man1" INSTALLSITEMAN3DIR="$PREFIX/share/man/man3" "$@"
}

# qnx_perl_xs_install: make install into the staging tree. perllocal.pod
# (every module install appends to it: a file two packages would share) and
# .packlist (it lists the DESTDIR paths) are not shipped.
qnx_perl_xs_install() {
  qnx_perl_xs_make DESTDIR="$STAGE" install && \
  find "$STAGE$PREFIX/lib/perl5" \( -name perllocal.pod -o -name .packlist \) -delete
}
