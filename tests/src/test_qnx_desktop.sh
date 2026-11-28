#!/bin/sh
# qnx-desktop (the meta packages qnx-desktop-core, -theme, -preferences,
# -utilities, -applications, -developer, -graphics, -extras and qnx-desktop,
# which depends on all of them): after "apk add qnx-desktop",
#  - every group left its member list (share/qnx-desktop/<group>.list) and
#    apk has every member installed -- a meta package is only as good as its
#    dependencies;
#  - every member's main program or library is there and loads: ldd resolves
#    every library of an ELF program or plugin, a script is executable, data
#    (icon themes, fonts, schemas, keyboard layouts) is in place, the Python
#    modules import;
#  - a member in a list that this test has no check for is a failure: a
#    component added to a recipe gets its check here;
#  - the session's start scripts (oss-desktop, the boot entry
#    qnx-desktop-start and the rc.local sample) pass "sh -n", and the boot
#    entry's --help names its options.
# Nothing is started: no Screen, no display, no daemon.
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL qnx_desktop (qnxport-timeout.sh missing)"; exit 1; }
echo "== qnx-desktop =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=${QNX_PORT_PREFIX:-/usr/local}
L=$P/lib
D=$P/share/qnx-desktop
T=$(mktemp -d /tmp/qnxport-qnxdesktop.XXXXXX) || { echo "FAIL qnx-desktop (mktemp)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

# apk on this target (target/apk-setup: a wrapper for the /data/oss root).
APK=""
for a in /data/oss/bin/apk "$P/bin/apk"; do [ -x "$a" ] && { APK=$a; break; }; done
installed() { [ -n "$APK" ] && "$APK" info -e "$1" >/dev/null 2>&1; }

# What is checked for each member: <member> <kind> <path under the prefix>
#   x  an ELF program or plugin: present, ldd resolves every library
#   s  a script or a link to one: present and executable
#   f  a file or a directory that is not empty
#   p  a Python module: python3 imports it (the path column is the module)
cat > checks <<'EOT'
oss-desktop s bin/oss-desktop
labwc x bin/labwc
swaybg x bin/swaybg
xfce4-panel x bin/xfce4-panel
xfce4-session x bin/xfce4-session
xfce4-settings x bin/xfsettingsd
xfce4-terminal x bin/xfce4-terminal
thunar x bin/thunar
xfce4-appfinder x bin/xfce4-appfinder
xfconf x bin/xfconf-query
garcon x lib/libgarcon-1.so.0
exo x bin/exo-open
tumbler x lib/tumbler-1/tumblerd
dbus x bin/dbus-daemon
libnotify x bin/notify-send
xwayland x bin/Xwayland
xkeyboard-config f share/xkeyboard-config-2/rules/evdev
xdg-utils s bin/xdg-open
shared-mime-info x bin/update-mime-database
desktop-file-utils x bin/desktop-file-validate
gsettings-desktop-schemas f share/glib-2.0/schemas/org.gnome.desktop.interface.gschema.xml
glib-networking x lib/gio/modules/libgiognutls.so
gnome-keyring x bin/gnome-keyring-daemon
adwaita-icon-theme f share/icons/Adwaita/index.theme
hicolor-icon-theme f share/icons/hicolor/index.theme
lxde-icon-theme f share/icons/nuoveXT2/index.theme
adwaita-fonts f share/fonts/adwaita-fonts
font-dejavu f share/fonts/font-dejavu
font-liberation f share/fonts/font-liberation
font-cantarell f share/fonts/font-cantarell
font-inter f share/fonts/font-inter
font-hack f share/fonts/font-hack
font-inconsolata f share/fonts/font-inconsolata
font-noto-sans-cjk f share/fonts/font-noto-sans-cjk
font-noto-emoji f share/fonts/font-noto-emoji
fontconfig x lib/libfontconfig.so.1
cups x sbin/cupsd
cups-filters x sbin/cups-browsed
ippusbxd x sbin/ippusbxd
avahi x sbin/avahi-daemon
chrony x sbin/chronyd
tzdata f share/zoneinfo
libpaper x bin/paper
geany x bin/geany
gthumb x bin/gthumb
mate-calc x bin/mate-calc
htop x bin/htop
ffmpegthumbnailer x bin/ffmpegthumbnailer
7zip x bin/7zz
zip x bin/zip
nano x bin/nano
dconf x bin/dconf
xfce4-notifyd x bin/xfce4-notifyd-config
xfdesktop x bin/xfdesktop
swayidle x bin/swayidle
evince x bin/evince
xdg-user-dirs x bin/xdg-user-dirs-update
xfce4-genmon-plugin x lib/xfce4/panel/plugins/libgenmon.so
swaylock x bin/swaylock
xarchiver x bin/xarchiver
epiphany x bin/epiphany
parole x bin/parole
vlc x bin/vlc
geany-plugins x lib/libgeanypluginutils.so.0
native-toolchain s bin/cc
cmake x bin/cmake
ninja x bin/ninja
meson s bin/meson
python3 x bin/python3
python3-pip p pip
python3-setuptools p setuptools
python3-wheel p wheel
python3-virtualenv p virtualenv
numpy p numpy
pygame p pygame
git x bin/git
gdb x bin/gdb
nodejs x bin/node
npm s bin/npm
gstreamer x bin/gst-launch-1.0
gst-plugins-base x lib/gstreamer-1.0/libgstplayback.so
gst-plugins-good x lib/gstreamer-1.0/libgstisomp4.so
gst-plugins-bad x lib/gstreamer-1.0/libgstmpegtsdemux.so
gst-plugins-ugly x lib/gstreamer-1.0/libgstasf.so
gst-libav x lib/gstreamer-1.0/libgstlibav.so
ffmpeg x bin/ffmpeg
mesa f opt/mesa/lib
webrtc-screencast x bin/webrtc-screencast
gtk-vnc x bin/gvncviewer
openssh x bin/ssh
sudo x bin/sudo
librsvg x lib/librsvg-2.so.2
oss-greeter x libexec/oss-greeter/oss-greeter
oss-network-plugin x lib/xfce4/panel/plugins/liboss-network.so
xfce4-alsa-plugin x lib/xfce4/panel/plugins/libalsa.so
xfce4-cpugraph-plugin x lib/xfce4/panel/plugins/libcpugraph.so
xfce4-screenshooter x bin/xfce4-screenshooter
xfce4-taskmanager x bin/xfce4-taskmanager
mousepad x bin/mousepad
oss-software x bin/oss-software
abiword x bin/abiword
gnumeric x bin/gnumeric
EOT

GROUPS_ALL="core theme preferences utilities applications developer graphics extras"
[ -n "$APK" ] && echo "  info apk: $APK" || echo "  info apk not found: the installed state of the packages is not asked, only their files"

# 1. the top-level package names its eight groups
if [ -f "$D/qnx-desktop.list" ]; then
  miss=""
  for g in $GROUPS_ALL; do grep -qx "qnx-desktop-$g" "$D/qnx-desktop.list" || miss="$miss $g"; done
  [ -z "$miss" ] && ok "qnx-desktop lists its 8 groups" || bad "qnx-desktop.list lacks:$miss"
  if [ -n "$APK" ]; then installed qnx-desktop && ok "apk: qnx-desktop is installed" || bad "apk: qnx-desktop is not installed"; fi
else bad "no $D/qnx-desktop.list (apk add qnx-desktop)"; fi

# 2. every group: its list, its members installed, each member's check
PY=$P/bin/python3
one() { # one <member>: run its check; says ok/FAIL itself
  m=$1
  set -- $(grep "^$m " checks)
  [ $# -eq 3 ] || { bad "$m: this test has no check for it (add one to the table)"; return; }
  k=$2; f=$P/$3
  case "$k" in
    x) if [ ! -f "$f" ]; then bad "$m: no $f"
       elif ldd "$f" > ldd.out 2>&1; then ok "$m: $3 loads (ldd)"
       else bad "$m: $3 does not load: $(grep -i 'not found\|cannot\|error' ldd.out | head -n 2)"; fi ;;
    s) [ -x "$f" ] && ok "$m: $3 is executable" || bad "$m: no executable $f" ;;
    f) if [ -d "$f" ]; then [ -n "$(ls "$f" 2>/dev/null)" ] && ok "$m: $3 is there" || bad "$m: $f is empty or missing"
       else [ -s "$f" ] && ok "$m: $3 is there" || bad "$m: no $f"; fi ;;
    p) if [ ! -x "$PY" ]; then bad "$m: no $PY to import $3"
       elif SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy PYGAME_HIDE_SUPPORT_PROMPT=1 qto 120 "$PY" -c "import $3" > py.out 2>&1; then ok "$m: python3 imports $3"
       else bad "$m: import $3: $(tail -n 1 py.out)"; fi ;;
    *) bad "$m: unknown kind $k" ;;
  esac
}
total=0
for g in $GROUPS_ALL; do
  lst=$D/$g.list
  if [ ! -f "$lst" ]; then bad "qnx-desktop-$g: no $lst"; continue; fi
  members=$(sed 's/#.*//' "$lst")
  n=0; notinst=""
  for m in $members; do
    n=$((n+1)); total=$((total+1))
    if [ -n "$APK" ]; then installed "$m" || notinst="$notinst $m"; fi
    one "$m"
  done
  [ $n -gt 0 ] || bad "qnx-desktop-$g: its list is empty"
  if [ -n "$APK" ]; then
    installed "qnx-desktop-$g" || notinst="$notinst qnx-desktop-$g"
    [ -z "$notinst" ] && ok "apk: qnx-desktop-$g and its $n members are installed" || bad "apk: not installed:$notinst"
  fi
done
echo "  info $total members in 8 groups"
[ $total -ge 80 ] && ok "all members listed ($total)" || bad "only $total members listed (82 expected)"

# 3. the session's scripts
for s in bin/oss-desktop bin/oss-desktop-menu bin/qnx-desktop-start share/qnx-desktop/rc.local.sample; do
  if [ ! -f "$P/$s" ]; then bad "no $P/$s"
  elif sh -n "$P/$s" 2> sh.err; then ok "sh -n $s"
  else bad "sh -n $s: $(head -n 2 sh.err)"; fi
done
[ -x "$P/bin/qnx-desktop-start" ] && ok "qnx-desktop-start is executable" || bad "qnx-desktop-start is not executable"
h=$(sh "$P/bin/qnx-desktop-start" --help 2>&1)
echo "$h" | grep -q -- "--install-hook" && echo "$h" | grep -q -- "--check" && ok "qnx-desktop-start --help names --install-hook and --check" || bad "qnx-desktop-start --help: $(echo "$h" | head -n 2)"
grep -q 'qnx-desktop-start' "$P/share/qnx-desktop/rc.local.sample" 2>/dev/null && ok "rc.local.sample calls qnx-desktop-start" || bad "rc.local.sample does not call qnx-desktop-start"

[ $fail -eq 0 ] && { echo "PASS qnx-desktop"; exit 0; }
echo "FAIL qnx-desktop ($fail check(s) failed)"; exit 1
