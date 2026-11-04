# tzdata: the IANA time zone database compiled into TZif files under
# share/zoneinfo (Area/City names), with zone.tab, zone1970.tab, iso3166.tab,
# tzdata.zi and leap-seconds.list, plus the target's zic and zdump. QNX's
# libc reads a TZif file when TZ names one by path (TZ=/usr/local/share/
# zoneinfo/Europe/Berlin, or TZ=:<path>): the whole history of a zone, not
# only the rules of a POSIX TZ string; Python's zoneinfo, ICU-less programs and
# anything that opens /usr/share/zoneinfo-style files use it the same way.
# QNX's own package list has 2025b; this is the IANA release current when it
# was ported (the data changes several times a year).
VERSION=2026f
TARBALL=https://data.iana.org/time-zones/releases/tzdb-2026f.tar.lz
SHA256=357829e3a16a78224d23f1ae2327704c1da1fd35f6769a7aac0682755fb9027a
BUILDSYS=custom
DEPS=""
# The zones zic compiles: the main data files (not backzone, which is the
# pre-1970 detail Alpine and QNX leave out too).
TZ_FILES="africa antarctica asia australasia europe northamerica southamerica etcetera backward factory"
do_configure() { :; }
do_build() {
  # tzcode's Makefile builds in the source tree: one copy for the build
  # machine's zic (it writes the data, so it has to run here) and one for the
  # target's programs.
  rm -rf "$B/host" "$B/target"; mkdir -p "$B"
  cp -a "$S" "$B/host" && cp -a "$S" "$B/target" || return 1
  make -C "$B/host" CC=gcc CFLAGS=-O2 zic || return 1
  # TZDIR: where the target's zdump and zic look for zone names by default.
  # HAVE_GETTEXT=0: the staged libintl.h (gettext) would otherwise switch on
  # message translation and a dependency on libintl for two small tools.
  # HAVE_STRFTIME_L=0: QNX has no locale_t / strftime_l; HAVE_GETRESUID=0:
  # nor getresuid/getresgid (zdump carries tzcode's own localtime.c).
  make -C "$B/target" CC="qcc -V$QCC_TARGET" AR="nto${QNX_CPU}-ar" CFLAGS="-O2 -D_QNX_SOURCE -DHAVE_GETTEXT=0 -DHAVE_STRFTIME_L=0 -DHAVE_GETRESUID=0" \
       TZDIR="$PREFIX/share/zoneinfo" zic zdump
}
do_install() {
  local z="$STAGE$PREFIX/share/zoneinfo"
  mkdir -p "$z" "$STAGE$PREFIX/bin"
  # -b fat: the transitions are written out in full up to 2037 rather than
  # left to the POSIX TZ footer, as Alpine and QNX build it -- readers that
  # only take the transition table (older libcs) get the right answers too.
  ( cd "$S" && "$B/host/zic" -b fat -d "$z" $TZ_FILES ) || return 1
  # posixrules: the rules a POSIX TZ string without a rule part ("EST5EDT")
  # takes, as tzcode-derived libcs (QNX's among them) look for it.
  ln -sf America/New_York "$z/posixrules"
  install -m644 "$S"/zone.tab "$S"/zone1970.tab "$S"/zonenow.tab "$S"/iso3166.tab \
                "$S"/tzdata.zi "$S"/leap-seconds.list "$z/"
  echo "$VERSION" > "$z/+VERSION"
  install -m755 "$B/target/zic" "$B/target/zdump" "$STAGE$PREFIX/bin/"
}
