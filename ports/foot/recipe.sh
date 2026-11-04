# foot: a fast, lightweight and minimalistic Wayland terminal emulator
# (fcft fonts, pixman rendering, a server/client mode, sixel images).
VERSION=1.23.1; GIT=https://codeberg.org/dnkl/foot.git; REF=1.23.1
DEPS="pixman wayland wayland-protocols libxkbcommon fontconfig utf8proc tllist fcft ncurses"
# The "foot" terminfo entries come with ncurses' terminfo database, so foot's
# own copy is not installed (they would be the same files twice).
RUNTIME_DEPS="ncurses"
BUILDSYS=meson
# - grapheme clustering with utf8proc; IME support; the colour themes;
# - no utmp logging (no libutempter on QNX), no man pages (scdoc), no tests.
MESON_ARGS_EXTRA=( -Ddocs=disabled -Dtests=false -Dterminfo=disabled -Ddefault-terminfo=foot
                   -Dgrapheme-clustering=enabled -Dime=true -Dthemes=true -Dutmp-backend=none )
