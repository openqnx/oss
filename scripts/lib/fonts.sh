# Helpers for font and other data-only components: nothing is compiled, the
# published archive is downloaded (SHA256 pinned in the recipe) and its files
# are installed. Recipe:
#   TARBALL=<url>  SHA256=...  BUILDSYS=custom  source "$WS/scripts/lib/fonts.sh"
#   do_fetch() { qnx_data_fetch; }  do_build() { :; }  do_install() { qnx_fonts_install <dir-name>; }
qnx_data_fetch() {
  local a="$WS/dist/${FETCH_NAME:-$(basename "$TARBALL")}"
  mkdir -p "$WS/dist" "$SRCROOT"
  [ -f "$a" ] || curl -fsSL -o "$a" "$TARBALL" || return 1
  echo "$SHA256  $a" | sha256sum -c --status || { echo "checksum mismatch: $a"; rm -f "$a"; return 1; }
  case "$a" in
    *.zip) ( cd "$SRCROOT" && unzip -q "$a" ) ;;
    *)     tar xf "$a" -C "$SRCROOT" ;;
  esac
}
# Every .ttf/.otf/.ttc in the archive, flat, into share/fonts/<name>.
qnx_fonts_install() {
  local d="$STAGE$PREFIX/share/fonts/$1"; mkdir -p "$d"
  find "$S" -type f \( -iname '*.ttf' -o -iname '*.otf' -o -iname '*.ttc' \) ${FONT_FILTER:+-path "$FONT_FILTER"} \
    -exec install -m644 {} "$d/" \;
  [ -n "$(ls "$d")" ]
}
