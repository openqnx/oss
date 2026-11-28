#!/bin/sh
# xdg-user-dirs: in a home directory made for the test. Checks:
# xdg-user-dirs-update --help prints the usage; the defaults file and the
# autostart entry are installed; the first run (C locale) creates the nine
# folders of user-dirs.defaults and writes ~/.config/user-dirs.dirs with
# $HOME-relative entries, and user-dirs.locale; xdg-user-dir prints a
# folder's path, and the home directory for a name it does not know; --set
# moves one entry to another path; a folder removed by the user is not
# created again on the next run (its entry falls back to $HOME);
# --dummy-output writes the file elsewhere and creates nothing; a run in
# German (when the catalogue is installed: de.mo) names the folders in
# German.
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL xdg-user-dirs (qnxport-timeout.sh missing)"; exit 1; }
echo "== xdg-user-dirs =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=${QNX_PORT_PREFIX:-/usr/local}
T=$(mktemp -d /tmp/qnxport-xdgud.XXXXXX) || { echo "FAIL xdg-user-dirs (mktemp)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
U="qto 30 $P/bin/xdg-user-dirs-update"
D="qto 30 $P/bin/xdg-user-dir"
unset XDG_CONFIG_DIRS XDG_DATA_DIRS LANGUAGE LC_MESSAGES
LC_ALL=C; LANG=C; export LC_ALL LANG

h=$($U --help 2>&1)
case "$h" in *"--force"*"--dummy-output"*"--set"*) ok "xdg-user-dirs-update --help: $h" ;; *) bad "--help: $h" ;; esac
[ -f "$P/etc/xdg/user-dirs.defaults" ] && [ -f "$P/etc/xdg/user-dirs.conf" ] && [ -f "$P/etc/xdg/autostart/xdg-user-dirs.desktop" ] \
  && ok "user-dirs.defaults, user-dirs.conf and the autostart entry are installed" || bad "files under $P/etc/xdg missing"

# --- first run ---
HOME=$T/home; XDG_CONFIG_HOME=$HOME/.config; export HOME XDG_CONFIG_HOME
mkdir -p "$HOME"
# started somewhere else on purpose: the configuration is found by its
# absolute path, not relative to the current directory
( cd / && $U ) > up.out 2>&1; rc=$?
n=0; miss=""
for d in Desktop Downloads Templates Public Documents Music Pictures Videos Projects; do
  [ -d "$HOME/$d" ] && n=$((n+1)) || miss="$miss $d"
done
[ $rc -eq 0 ] && [ $n -eq 9 ] && ok "first run created the 9 folders" || bad "first run: rc=$rc, $n of 9 (missing:$miss) $(head -n 3 up.out)"
F=$XDG_CONFIG_HOME/user-dirs.dirs
grep -q '^XDG_DESKTOP_DIR="\$HOME/Desktop"$' "$F" 2>/dev/null && grep -q '^XDG_DOWNLOAD_DIR="\$HOME/Downloads"$' "$F" \
  && grep -q '^XDG_PICTURES_DIR="\$HOME/Pictures"$' "$F" && ok "user-dirs.dirs has \$HOME-relative entries" || bad "user-dirs.dirs: $(grep -v '^#' "$F" 2>&1 | head -n 4)"
[ "$(cat "$XDG_CONFIG_HOME/user-dirs.locale" 2>/dev/null)" = "C" ] && ok "user-dirs.locale records the locale (C)" || bad "user-dirs.locale: $(cat "$XDG_CONFIG_HOME/user-dirs.locale" 2>&1)"

# --- lookup ---
v=$($D DOWNLOAD 2>&1); [ "$v" = "$HOME/Downloads" ] && ok "xdg-user-dir DOWNLOAD: $v" || bad "xdg-user-dir DOWNLOAD: $v"
v=$($D DESKTOP 2>&1); [ "$v" = "$HOME/Desktop" ] && ok "xdg-user-dir DESKTOP" || bad "xdg-user-dir DESKTOP: $v"
v=$($D NOSUCHDIR 2>&1); [ "$v" = "$HOME" ] && ok "an unknown name gives the home directory" || bad "xdg-user-dir NOSUCHDIR: $v"

# --- --set ---
mkdir -p "$HOME/Tunes"
$U --set MUSIC "$HOME/Tunes" > set.out 2>&1
v=$($D MUSIC 2>&1); [ "$v" = "$HOME/Tunes" ] && grep -q '^XDG_MUSIC_DIR="\$HOME/Tunes"$' "$F" && ok "--set MUSIC: $v" || bad "--set MUSIC: $v $(head -n 2 set.out)"
v=$($D DOWNLOAD 2>&1); [ "$v" = "$HOME/Downloads" ] && ok "--set left the other entries alone" || bad "after --set, DOWNLOAD: $v"

# --- a removed folder stays removed ---
rmdir "$HOME/Videos"
$U > up2.out 2>&1
v=$($D VIDEOS 2>&1)
[ ! -d "$HOME/Videos" ] && [ "${v%/}" = "$HOME" ] && ok "a removed folder is not created again; its entry is the home directory" || bad "after removal: VIDEOS=$v, exists: $(ls -d "$HOME/Videos" 2>&1)"

# --- --dummy-output ---
HOME=$T/home2; XDG_CONFIG_HOME=$HOME/.config; mkdir -p "$HOME"
$U --dummy-output "$T/dummy.dirs" > dummy.out 2>&1
grep -q '^XDG_DOCUMENTS_DIR="\$HOME/Documents"$' "$T/dummy.dirs" 2>/dev/null && [ ! -d "$HOME/Documents" ] && [ ! -f "$XDG_CONFIG_HOME/user-dirs.dirs" ] \
  && ok "--dummy-output wrote the list and created nothing" || bad "--dummy-output: $(head -n 3 "$T/dummy.dirs" 2>&1) $(ls "$HOME")"

# --- translated names ---
if [ -f "$P/share/locale/de/LC_MESSAGES/xdg-user-dirs.mo" ]; then
  ok "translations installed (de.mo)"
  HOME=$T/home3; XDG_CONFIG_HOME=$HOME/.config; mkdir -p "$HOME"
  LC_ALL=de_DE.UTF-8 LANG=de_DE.UTF-8 LANGUAGE=de $U > de.out 2>&1
  if [ -d "$HOME/Bilder" ] && [ -d "$HOME/Dokumente" ]; then ok "German session: Bilder, Dokumente, $(LC_ALL=C $D DESKTOP)"
  elif [ -d "$HOME/Pictures" ]; then echo "  info no de_DE.UTF-8 locale on this target: the folders keep their English names"
  else bad "German run created nothing: $(head -n 3 de.out)"; fi
else bad "no $P/share/locale/de/LC_MESSAGES/xdg-user-dirs.mo"; fi

[ $fail -eq 0 ] && { echo "PASS xdg-user-dirs"; exit 0; }
echo "FAIL xdg-user-dirs ($fail check(s) failed)"; exit 1
