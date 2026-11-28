#!/bin/sh
# Xarchiver (GTK 3 archive manager): under a headless labwc (pixman
# renderer). Checks: --version names 0.5.4.27; --help lists the
# non-interactive modes; --info reports the archivers found in PATH (tar,
# and 7z when 7zz is installed); "xarchiver --extract-to=DIR ARCHIVE" -- the
# mode the file manager's "Extract here" runs -- unpacks a .tar.gz, a
# .tar.xz and a .zip made by the test, with the files' contents and a
# sub-directory intact, and quits by itself; --ensure-directory extracts into
# a directory named after the archive; the desktop entry and the Thunar
# archive plugin script are installed.
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL xarchiver (qnxport-timeout.sh missing)"; exit 1; }
echo "== xarchiver =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=${QNX_PORT_PREFIX:-/usr/local}
T=$(mktemp -d /tmp/qnxport-xarchiver.XXXXXX) || { echo "FAIL xarchiver (mktemp)"; exit 1; }
LWP=""
trap '[ -n "$LWP" ] && kill $LWP 2>/dev/null; cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
# The archivers of this tree first: Xarchiver takes the first "tar" in PATH
# and gives it GNU tar's options.
PATH=$P/bin:$PATH; export PATH
X=$P/bin/xarchiver

HOME=$T/home; XDG_CONFIG_HOME=$HOME/.config; XDG_CACHE_HOME=$HOME/.cache
export HOME XDG_CONFIG_HOME XDG_CACHE_HOME
mkdir -p "$XDG_CONFIG_HOME" "$XDG_CACHE_HOME"
RT=$T/rt; mkdir -m 700 "$RT"
XDG_RUNTIME_DIR=$RT WLR_BACKENDS=headless WLR_RENDERER=pixman "$P/bin/labwc" > labwc.log 2>&1 &
LWP=$!
i=0; while [ ! -S "$RT/wayland-0" ] && [ $i -lt 150 ]; do sleep 0.1; i=$((i+1)); done
[ -S "$RT/wayland-0" ] && ok "labwc up (headless)" || bad "labwc: $(tail -n 2 labwc.log)"
XDG_RUNTIME_DIR=$RT; WAYLAND_DISPLAY=wayland-0; GDK_BACKEND=wayland; export XDG_RUNTIME_DIR WAYLAND_DISPLAY GDK_BACKEND

v=$(qto 60 "$X" --version 2>&1 | grep -m1 -i "^xarchiver")
echo "  info $v"
case "$v" in *"0.5.4.27"*) ok "xarchiver --version: 0.5.4.27" ;; *) bad "xarchiver --version: $v" ;; esac
h=$(qto 60 "$X" --help 2>&1)
echo "$h" | grep -q -- "--extract-to" && echo "$h" | grep -q -- "--ensure-directory" && echo "$h" | grep -q -- "--compress" \
  && ok "xarchiver --help lists --extract-to, --ensure-directory and --compress" || bad "xarchiver --help: $(echo "$h" | head -n 3)"
inf=$(qto 60 "$X" --info 2>&1)
echo "$inf" | grep -q "tar" && ok "xarchiver --info: tar archives are handled" || bad "xarchiver --info: $(echo "$inf" | head -n 5)"
echo "  info types: $(echo "$inf" | grep -c . ) lines; 7z: $(echo "$inf" | grep -c '^7z\|[ ,]7z')"

# --- the archives ---
mkdir -p in/sub
echo "first file of the archive" > in/a.txt
i=0; : > in/sub/b.dat; while [ $i -lt 200 ]; do echo "line $i of the second file, in a sub-directory" >> in/sub/b.dat; i=$((i+1)); done
printf 'name with spaces\n' > "in/c d.txt"
sum_in=$(cd in && cat a.txt sub/b.dat "c d.txt" | cksum)
( cd in && tar -czf ../t.tar.gz a.txt sub "c d.txt" ) 2> mk.err || bad "tar -czf: $(head -n 2 mk.err)"
( cd in && tar -cJf ../t.tar.xz a.txt sub "c d.txt" ) 2> mk.err || bad "tar -cJf: $(head -n 2 mk.err)"
have_zip=0
[ -x "$P/bin/zip" ] && ( cd in && qto 30 "$P/bin/zip" -q -r ../t.zip a.txt sub "c d.txt" ) > mk.out 2>&1 && have_zip=1

check_tree() {  # $1 = directory holding a.txt, sub/b.dat, "c d.txt"
  [ -f "$1/a.txt" ] && [ -f "$1/sub/b.dat" ] && [ -f "$1/c d.txt" ] && [ "$(cd "$1" && cat a.txt sub/b.dat "c d.txt" | cksum)" = "$sum_in" ]
}
extract() {     # $1 = archive, $2 = label
  mkdir "out.$2"
  qto 90 "$X" --extract-to="$T/out.$2" "$T/$1" > "x.$2.log" 2>&1; rc=$?
  if [ $rc -eq 0 ] && check_tree "out.$2"; then ok "--extract-to: $1 unpacked, contents match, exit 0"
  elif [ $rc -eq 124 ]; then bad "--extract-to $1: did not quit (timeout); $(tail -n 2 "x.$2.log")"
  else bad "--extract-to $1: rc=$rc, $(ls -R "out.$2" 2>/dev/null | head -n 6 | tr '\n' ' ') $(tail -n 2 "x.$2.log")"; fi
}
extract t.tar.gz gz
extract t.tar.xz xz
if [ $have_zip -eq 1 ]; then
  if [ -x "$P/bin/unzip" ] || [ -x "$P/bin/7zz" ] || command -v unzip > /dev/null 2>&1; then extract t.zip zip
  else echo "  info neither unzip nor 7zz installed: .zip extraction skipped"; fi
else echo "  info zip not installed: .zip check skipped"; fi

# --- --ensure-directory: into a directory named after the archive ---
mkdir ens && cp t.tar.gz ens/bundle.tar.gz
( cd ens && qto 90 "$X" --ensure-directory "$T/ens/bundle.tar.gz" > ../ens.log 2>&1 ); rc=$?
d=$(ls -d ens/*/ 2>/dev/null | head -n 1)
[ $rc -eq 0 ] && [ -n "$d" ] && check_tree "${d%/}" && ok "--ensure-directory: extracted into ${d%/}" || bad "--ensure-directory: rc=$rc, $(ls ens | tr '\n' ' ') $(tail -n 2 ens.log)"

[ -f "$P/share/applications/xarchiver.desktop" ] && grep -q "application/x-compressed-tar" "$P/share/applications/xarchiver.desktop" \
  && ok "desktop entry lists the archive MIME types" || bad "share/applications/xarchiver.desktop"
[ -x "$P/libexec/thunar-archive-plugin/xarchiver.tap" ] && ok "Thunar archive plugin script installed" || bad "no libexec/thunar-archive-plugin/xarchiver.tap"
kill -0 $LWP 2>/dev/null || bad "labwc died: $(tail -n 3 labwc.log)"

[ $fail -eq 0 ] && { echo "PASS xarchiver"; exit 0; }
echo "FAIL xarchiver ($fail check(s) failed)"; exit 1
