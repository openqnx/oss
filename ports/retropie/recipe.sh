# RetroPie for QNX: the retro gaming stack qnx-ports assembled -- RetroArch
# (frontend), EmulationStation (game browser), libretro cores and OpenTTD --
# plus the configuration and start-up scripts from qnx-ports/build-files
# ports/RetroPie (files/). This component installs only those files; the
# programs are the components it depends on.
VERSION=0.3
DEPS="retroarch emulationstation openttd libretro-2048 libretro-samples libretro-mrboom libretro-retro8 libretro-fake08"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/retropie/files/." "$SRCROOT/"; }
do_build() { :; }
do_install() { install -d "$STAGE$PREFIX/share/retropie" && cp -a "$S/." "$STAGE$PREFIX/share/retropie/"; }
