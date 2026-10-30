#!/bin/sh
# GraphicsMagick: gm makes, converts, resizes and identifies images --
# known sizes and formats, and a pixel's colour read back.
echo "== graphicsmagick =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/gm.XXXXXX); cd "$T" || exit 1
echo "  info $(gm version 2>&1 | head -n 1)"
gm convert -size 64x32 xc:red red.png && [ "$(gm identify -format '%m %wx%h' red.png)" = "PNG 64x32" ] && ok "xc:red -> PNG 64x32" || bad "make PNG"
gm convert red.png -resize 50% red.jpg && [ "$(gm identify -format '%m %wx%h' red.jpg)" = "JPEG 32x16" ] && ok "resize 50% -> JPEG 32x16" || bad "resize to JPEG"
gm convert red.png -rotate 90 tall.tif && [ "$(gm identify -format '%m %wx%h' tall.tif)" = "TIFF 32x64" ] && ok "rotate 90 -> TIFF 32x64" || bad "rotate to TIFF"
gm convert red.png webp.webp 2>/dev/null && [ "$(gm identify -format '%m' webp.webp)" = WEBP ] && ok "PNG -> WebP" || bad "WebP"
p=$(gm convert red.png -crop 1x1+10+10 txt:- 2>/dev/null | tail -n 1)
echo "  info pixel (10,10): $p"
echo "$p" | grep -qiE "red|#FF0000|65535, *0, *0|255, *0, *0" && ok "pixel is red" || bad "pixel is red"
cd /; rm -rf "$T"
[ $fail -eq 0 ] && { echo "PASS graphicsmagick"; exit 0; }
echo "FAIL graphicsmagick ($fail check(s) failed)"; exit 1
