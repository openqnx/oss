#!/bin/sh
# adwaita-icon-theme: the theme GTK looks icons up in -- its index names
# the theme and inherits hicolor, the icons GTK's own widgets ask for exist
# (symbolic, scalable), and the cursor theme is complete enough for a
# compositor (default, pointer, text, wait).
echo "== adwaita-icon-theme =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
D=/usr/local/share/icons/Adwaita
grep -q "^Name=Adwaita" "$D/index.theme" && grep -q "^Inherits=.*hicolor" "$D/index.theme" && ok "index.theme: Adwaita, inherits hicolor" || bad "index.theme"
for i in actions/document-open-symbolic places/folder-symbolic ui/window-close-symbolic status/dialog-warning-symbolic; do
  [ -f "$D/symbolic/$i.svg" ] && ok "symbolic/$i.svg" || bad "symbolic/$i.svg"
done
n=$(find "$D" -name '*.svg' | wc -l); [ "$n" -gt 500 ] && ok "$n SVG icons" || bad "only $n SVG icons"
for c in default pointer text wait; do [ -e "$D/cursors/$c" ] && ok "cursor $c" || bad "cursor $c"; done
grep -q "<svg" "$D/symbolic/places/folder-symbolic.svg" && ok "icons are SVG documents" || bad "SVG content"
[ $fail -eq 0 ] && { echo "PASS adwaita-icon-theme"; exit 0; }
echo "FAIL adwaita-icon-theme ($fail check(s) failed)"; exit 1
