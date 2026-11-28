#!/bin/sh
# Evince (GNOME document viewer): a two-page PDF and a comic book archive
# are made by the test. Checks: --version names 48.5 and --help lists the
# viewer's options; the PDF, TIFF and comics backends are installed and their
# modules load; the org.gnome.Evince GSettings schema is compiled in;
# evince-thumbnailer (no display) renders page 1 of the PDF into a PNG of the
# asked size, with the page's dark rectangle where the PDF draws it, and
# refuses a file that is not a document; it opens the .cbz through
# libarchive; under a headless labwc and a private session bus the viewer
# opens the PDF: evinced (started by D-Bus activation) knows the document's
# owner, the owner's org.gnome.evince.Application lists one window, which
# exports org.gnome.evince.Window, and a second "evince <same file>" is handed to
# the running instance instead of opening another.
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL evince (qnxport-timeout.sh missing)"; exit 1; }
echo "== evince =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=${QNX_PORT_PREFIX:-/usr/local}
T=$(mktemp -d /tmp/qnxport-evince.XXXXXX) || { echo "FAIL evince (mktemp)"; exit 1; }
DBP=""; LWP=""; EP=""
trap '[ -n "$EP" ] && kill $EP 2>/dev/null; [ -n "$DBP" ] && kill $DBP 2>/dev/null; [ -n "$LWP" ] && kill $LWP 2>/dev/null; cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
G="qto 30 $P/bin/gdbus"

HOME=$T/home; XDG_CONFIG_HOME=$HOME/.config; XDG_CACHE_HOME=$HOME/.cache; XDG_DATA_HOME=$HOME/.local/share
export HOME XDG_CONFIG_HOME XDG_CACHE_HOME XDG_DATA_HOME
mkdir -p "$XDG_CONFIG_HOME" "$XDG_CACHE_HOME" "$XDG_DATA_HOME"
# Settings stay in memory: nothing of the test reaches a real dconf database.
GSETTINGS_BACKEND=memory; export GSETTINGS_BACKEND

# --- a two-page PDF (US Letter): page 1 has a black rectangle in its upper
# left quarter and a line of text; the cross-reference table is left to the
# reader to rebuild, which poppler does. ---
C1='0 0 0 rg
0 396 306 396 re f
BT /F1 24 Tf 330 100 Td (QNX port page one) Tj ET'
C2='BT /F1 24 Tf 100 700 Td (second page) Tj ET'
{
  printf '%%PDF-1.4\n'
  printf '1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n'
  printf '2 0 obj\n<< /Type /Pages /Kids [3 0 R 6 0 R] /Count 2 >>\nendobj\n'
  printf '3 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] /Contents 4 0 R /Resources << /Font << /F1 5 0 R >> >> >>\nendobj\n'
  printf '4 0 obj\n<< /Length %d >>\nstream\n%s\nendstream\nendobj\n' "${#C1}" "$C1"
  printf '5 0 obj\n<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>\nendobj\n'
  printf '6 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] /Contents 7 0 R /Resources << /Font << /F1 5 0 R >> >> >>\nendobj\n'
  printf '7 0 obj\n<< /Length %d >>\nstream\n%s\nendstream\nendobj\n' "${#C2}" "$C2"
  printf '8 0 obj\n<< /Title (QNX port test document) >>\nendobj\n'
  printf 'trailer\n<< /Root 1 0 R /Info 8 0 R /Size 9 >>\n%%%%EOF\n'
} > t.pdf

EV=$P/bin/evince
v=$(qto 60 "$EV" --version 2>&1 | grep -m1 "Document Viewer")
echo "  info $v"
case "$v" in *"48.5"*) ok "evince --version: 48.5" ;; *) bad "evince --version: $v" ;; esac

# --- backends ---
BD=$(ls -d "$P"/lib/evince/*/backends 2>/dev/null | head -n 1)
n=0; nb=0
for b in pdf tiff comics; do
  if [ -f "$BD/${b}document.evince-backend" ] && [ -f "$BD/lib${b}document.so" ]; then
    n=$((n+1)); ldd "$BD/lib${b}document.so" > ldd.out 2>&1 || { nb=$((nb+1)); echo "  info $b: $(grep -i 'not found\|cannot' ldd.out | head -n 2)"; }
  else echo "  info backend $b not installed in $BD"; fi
done
[ $n -eq 3 ] && [ $nb -eq 0 ] && ok "PDF, TIFF and comics backends installed, modules load (ldd)" || bad "backends: $n of 3 found, $nb do not load"
if [ -x "$P/bin/gsettings" ]; then
  qto 30 "$P/bin/gsettings" list-schemas 2>/dev/null | grep -q '^org.gnome.Evince$' && ok "GSettings schema org.gnome.Evince is compiled in" \
    || bad "schema org.gnome.Evince not in gschemas.compiled (run glib-compile-schemas $P/share/glib-2.0/schemas)"
fi

# --- thumbnailer: renders without a display ---
TH=$P/bin/evince-thumbnailer
qto 60 "$TH" -s 256 "$T/t.pdf" "$T/t.png" > th.out 2>&1; rc=$?
hd=$(head -c 8 t.png 2>/dev/null | od -An -tx1 | tr -d ' \n')
if [ $rc -eq 0 ] && [ "$hd" = "89504e470d0a1a0a" ]; then
  # IHDR: width and height, big-endian, at offsets 16 and 20
  w=$(od -An -tu1 -j16 -N4 t.png | awk '{print $1*16777216+$2*65536+$3*256+$4}')
  hh=$(od -An -tu1 -j20 -N4 t.png | awk '{print $1*16777216+$2*65536+$3*256+$4}')
  echo "  info thumbnail ${w}x${hh}, $(wc -c < t.png | tr -d ' ') bytes"
  # Letter is 612x792: the longer side becomes 256, the other 198 (+-1)
  [ "$hh" = "256" ] && [ "$w" -ge 197 ] && [ "$w" -le 199 ] && ok "evince-thumbnailer: page 1 as a ${w}x${hh} PNG" || bad "thumbnail size: ${w}x${hh}"
else bad "evince-thumbnailer: rc=$rc $(head -n 3 th.out)"; fi
# The pixels, through gdk-pixbuf's own tool when it is there: the upper left
# quarter is black, the upper right white.
if [ -x "$P/bin/gdk-pixbuf-pixdata" ] && [ -s t.png ]; then
  qto 30 "$P/bin/gdk-pixbuf-pixdata" t.png t.pixdata > /dev/null 2>&1
  if [ -s t.pixdata ]; then
    # GdkPixdata: a 24-byte header (rowstride at offset 12, big-endian), then the rows
    rs=$(od -An -tu1 -j12 -N4 t.pixdata | awk '{print $1*16777216+$2*65536+$3*256+$4}')
    ch=$(( rs / w ))
    px() { od -An -tu1 -j$(( 24 + $2 * rs + $1 * ch )) -N3 t.pixdata | tr -s ' ' | sed 's/^ //; s/ $//'; }
    a=$(px 40 40); b=$(px 150 40)
    echo "  info pixel(40,40)=$a pixel(150,40)=$b ($ch bytes/pixel)"
    [ "$a" = "0 0 0" ] && [ "$b" = "255 255 255" ] && ok "rendered page: black rectangle upper left, white upper right" || bad "rendered pixels: $a / $b"
  fi
fi
echo "this is not a document" > bad.pdf
qto 60 "$TH" -s 64 "$T/bad.pdf" "$T/bad.png" > /dev/null 2>&1 && bad "thumbnailer accepted a text file as PDF" || ok "thumbnailer refuses a file that is not a PDF"

# --- comics backend: a .cbz of two PNG pages (the thumbnail just made) ---
if [ -x "$P/bin/zip" ] && [ -s t.png ]; then
  cp t.png 001.png; cp t.png 002.png
  qto 30 "$P/bin/zip" -q book.cbz 001.png 002.png > zip.out 2>&1
  qto 60 "$TH" -s 128 "$T/book.cbz" "$T/cb.png" > cb.out 2>&1
  [ "$(head -c 4 cb.png 2>/dev/null | od -An -tx1 | tr -d ' \n')" = "89504e47" ] && ok "comics backend: thumbnail of a .cbz (libarchive)" || bad "cbz thumbnail: $(head -n 3 cb.out zip.out)"
else echo "  info zip not installed: .cbz check skipped"; fi

# --- the viewer, on a headless compositor ---
RT=$T/rt; mkdir -m 700 "$RT"
XDG_RUNTIME_DIR=$RT WLR_BACKENDS=headless WLR_RENDERER=pixman "$P/bin/labwc" > labwc.log 2>&1 &
LWP=$!
i=0; while [ ! -S "$RT/wayland-0" ] && [ $i -lt 150 ]; do sleep 0.1; i=$((i+1)); done
[ -S "$RT/wayland-0" ] && ok "labwc up (headless)" || bad "labwc: $(tail -n 2 labwc.log)"
XDG_RUNTIME_DIR=$RT; WAYLAND_DISPLAY=wayland-0; GDK_BACKEND=wayland; export XDG_RUNTIME_DIR WAYLAND_DISPLAY GDK_BACKEND
if [ -x "$P/bin/dbus-daemon" ]; then
  DBUS_SESSION_BUS_ADDRESS="unix:path=$T/bus"; export DBUS_SESSION_BUS_ADDRESS
  "$P/bin/dbus-daemon" --config-file="$P/share/dbus-1/session.conf" --address="$DBUS_SESSION_BUS_ADDRESS" --nofork --nopidfile > dbus.log 2>&1 &
  DBP=$!
  i=0; while [ $i -lt 10 ] && [ ! -S "$T/bus" ]; do sleep 1; i=$((i+1)); done
  [ -S "$T/bus" ] && ok "private D-Bus session bus" || bad "dbus-daemon: $(head -n 2 dbus.log)"
else bad "dbus-daemon not installed"; fi

h=$(qto 60 "$EV" --help 2>&1)
echo "$h" | grep -q -- "--page-index" && echo "$h" | grep -q -- "--presentation" && ok "evince --help lists --page-index and --presentation" || bad "evince --help: $(echo "$h" | head -n 3)"

"$EV" "$T/t.pdf" > evince.log 2>&1 &
EP=$!
URI="file://$T/t.pdf"
i=0; own=""
while [ $i -lt 40 ]; do
  r=$($G call --session --dest org.gnome.evince.Daemon --object-path /org/gnome/evince/Daemon --method org.gnome.evince.Daemon.FindDocument "$URI" false 2>&1)
  own=$(echo "$r" | sed -n "s/^('\(:[0-9.]*\)',)$/\1/p")
  [ -n "$own" ] && break
  kill -0 $EP 2>/dev/null || break
  sleep 1; i=$((i+1))
done
[ -n "$own" ] && ok "evinced (D-Bus activated) knows the open document: owner $own" || bad "FindDocument: $r; $(tail -n 4 evince.log)"
if [ -n "$own" ]; then
  wl=$($G call --session --dest "$own" --object-path /org/gnome/evince/Evince --method org.gnome.evince.Application.GetWindowList 2>&1)
  nw=$(echo "$wl" | grep -o "/org/gnome/evince/Window/[0-9]*" | wc -l | tr -d ' ')
  [ "$nw" = "1" ] && ok "the viewer has one window: $(echo "$wl" | grep -o "/org/gnome/evince/Window/[0-9]*")" || bad "GetWindowList: $wl"
  wp=$(echo "$wl" | grep -o "/org/gnome/evince/Window/[0-9]*" | head -n 1)
  i=0; tt=""
  while [ $i -lt 10 ] && [ -n "$wp" ]; do
    tt=$($G introspect --session --dest "$own" --object-path "$wp" 2>&1)
    echo "$tt" | grep -q "org.gnome.evince.Window" && break
    sleep 1; i=$((i+1))
  done
  echo "$tt" | grep -q "SyncView" && ok "org.gnome.evince.Window exported for the document (loaded)" || bad "window interface: $(echo "$tt" | head -n 4)"
  # a second invocation with the same file goes to the running viewer
  qto 30 "$EV" "$T/t.pdf" > second.log 2>&1; rc=$?
  wl=$($G call --session --dest "$own" --object-path /org/gnome/evince/Evince --method org.gnome.evince.Application.GetWindowList 2>&1)
  nw=$(echo "$wl" | grep -o "/org/gnome/evince/Window/[0-9]*" | wc -l | tr -d ' ')
  [ $rc -eq 0 ] && [ "$nw" = "1" ] && ok "a second evince with the same file is handed to the running one" || bad "second instance: rc=$rc windows=$nw $(head -n 2 second.log)"
fi
kill -0 $EP 2>/dev/null && ok "evince still running with the document" || bad "evince exited: $(tail -n 5 evince.log)"
kill -0 $LWP 2>/dev/null || bad "labwc died: $(tail -n 3 labwc.log)"

[ $fail -eq 0 ] && { echo "PASS evince"; exit 0; }
echo "FAIL evince ($fail check(s) failed)"; exit 1
