# openttd-opengfx: OpenGFX, the free base graphics set for OpenTTD (no original game data needed).
# Data only: the published archive (a .tar inside a .zip) is unpacked into
# share/games/openttd/baseset, where OpenTTD looks for base sets.
VERSION=8.0
TARBALL=https://cdn.openttd.org/opengfx-releases/8.0/opengfx-8.0-all.zip
SHA256=43a0c1dabf39cb865394f3a6cc36d4da5c10ecfaaf55652043104806810903be
DEPS=""
RUNTIME_DEPS="openttd"
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_configure() { :; }
do_build() { :; }
do_install() {
  local d="$STAGE$PREFIX/share/games/openttd/baseset"; mkdir -p "$d"
  local t; t=$(ls "$S"/*.tar | head -n 1); [ -f "$t" ] || return 1
  tar xf "$t" -C "$d"
}
