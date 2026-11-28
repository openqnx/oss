# xdg-user-dirs: the well-known folders of a home directory (Desktop,
# Downloads, Documents, Music, Pictures, Videos, Templates, Public, Projects).
# xdg-user-dirs-update creates them at the first login, named in the user's
# language, and records them in ~/.config/user-dirs.dirs, which GLib
# (g_get_user_special_dir), the file manager's side pane, file choosers and
# the desktop read; xdg-user-dir prints one of them. etc/xdg/autostart runs
# the update when a session starts.
VERSION=0.20; GIT=https://gitlab.freedesktop.org/xdg/xdg-user-dirs.git; REF=v0.20
# gettext: libintl (the translated folder names) and msgfmt at build time;
# libiconv: the folder names are converted from UTF-8 to the configured
# filename encoding, and QNX's libc has no iconv.
DEPS="gettext libiconv"
BUILDSYS=meson
# No man pages: they are made with xsltproc from DocBook stylesheets named by
# URL. The translations are installed.
MESON_ARGS_EXTRA=( -Ddocs=false -Dnls=true )
# The systemd user unit is installed without an option; QNX has no systemd
# (the XDG autostart entry does the same job).
do_install() { DESTDIR="$STAGE" ninja -C "$B" install && rm -rf "$STAGE$PREFIX/lib/systemd"; }
