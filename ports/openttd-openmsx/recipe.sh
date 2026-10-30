# openttd-openmsx: OpenMSX, the free base music set for OpenTTD.
# Data only: the published archive (a .tar inside a .zip) is unpacked into
# share/games/openttd/baseset, where OpenTTD looks for base sets.
VERSION=0.4.2
TARBALL=https://cdn.openttd.org/openmsx-releases/0.4.2/openmsx-0.4.2-all.zip
SHA256=5a4277a2e62d87f2952ea5020dc20fb2f6ffafdccf9913fbf35ad45ee30ec762
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
