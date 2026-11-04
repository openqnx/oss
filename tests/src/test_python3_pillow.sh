#!/bin/sh
# Pillow: images drawn with known pixels, written and read back in each
# format whose library is linked (PNG/zlib, JPEG, TIFF, WebP, JPEG 2000 --
# lossless ones must match exactly), resizing, rotation, filters, colour
# conversion, an ICC transform (LittleCMS), text drawn with FreeType through
# libraqm layout, and the features Pillow reports as compiled in.
echo "== python3-pillow =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pillow.XXXXXX") || { echo "FAIL python3-pillow (no temporary directory)"; exit 1; }
cd "$T" || exit 1
cat > t.py <<'P'
import sys, os, io
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def info(s): print("  info " + s)
def run(name, fn):
    try: fn()
    except Exception as e:
        import traceback; traceback.print_exc(limit=3, file=sys.stdout)
        check(False, "%s: %r" % (name, e))
import PIL
from PIL import Image, ImageDraw, ImageFilter, ImageOps, ImageChops, features
info("Pillow %s" % PIL.__version__)
def feats():
    for f in ("zlib", "jpg", "libtiff", "freetype2", "littlecms2", "jpg_2000", "raqm"):
        check(bool(features.check(f)), "feature %s compiled in" % f)
    check(features.check_module("webp"), "WebP module")
def mk():
    im = Image.new("RGB", (64, 48), (255, 255, 255))
    d = ImageDraw.Draw(im)
    d.rectangle([8, 8, 23, 23], fill=(255, 0, 0)); d.ellipse([32, 8, 55, 31], fill=(0, 0, 255))
    d.line([0, 47, 63, 47], fill=(0, 128, 0))
    return im
def draw():
    im = mk()
    check(im.getpixel((10, 10)) == (255, 0, 0) and im.getpixel((43, 19)) == (0, 0, 255) and im.getpixel((60, 2)) == (255, 255, 255), "drawn pixels")
    check(im.getpixel((5, 47)) == (0, 128, 0), "line on the last row")
    h = im.convert("L").histogram(); check(sum(h) == 64 * 48, "histogram counts every pixel")
def formats():
    im = mk()
    for fmt, ext, lossless in (("PNG", "png", True), ("TIFF", "tif", True), ("WEBP", "webp", None), ("JPEG2000", "j2k", None), ("JPEG", "jpg", False), ("BMP", "bmp", True), ("GIF", "gif", None)):
        kw = {"lossless": True} if fmt == "WEBP" else ({"irreversible": False} if fmt == "JPEG2000" else {})
        if fmt == "TIFF": kw = {"compression": "tiff_deflate"}
        im.save("i." + ext, fmt, **kw)
        back = Image.open("i." + ext); back.load(); back = back.convert("RGB")
        if fmt in ("PNG", "TIFF", "BMP") or fmt in ("WEBP", "JPEG2000"):
            same = ImageChops.difference(im, back).getbbox() is None
            check(back.size == (64, 48) and same, "%s round trip is exact" % fmt)
        elif fmt == "GIF":
            check(back.size == (64, 48) and back.getpixel((10, 10)) == (255, 0, 0), "GIF round trip keeps the palette colours")
        else:
            r, g, b = back.getpixel((15, 15))
            check(back.size == (64, 48) and r > 200 and g < 60 and b < 60, "JPEG round trip close to the original")
def ops():
    im = mk()
    check(im.resize((32, 24), Image.NEAREST).getpixel((5, 5)) == (255, 0, 0), "resize (nearest)")
    r = im.rotate(90, expand=True); check(r.size == (48, 64) and r.getpixel((10, 53)) == (255, 0, 0), "rotate 90 with expand")
    check(ImageOps.mirror(im).getpixel((63 - 10, 10)) == (255, 0, 0), "mirror")
    b = Image.new("L", (9, 9), 0); b.putpixel((4, 4), 255)
    bl = b.filter(ImageFilter.BoxBlur(1)); check(bl.getpixel((4, 4)) == 28 and bl.getpixel((3, 3)) == 28 and bl.getpixel((0, 0)) == 0, "box blur spreads 255 over 9 pixels")
    check(Image.new("RGB", (1, 1), (255, 0, 0)).convert("L").getpixel((0, 0)) == 76, "RGB red to L: 76 (ITU-R 601)")
    check(Image.new("RGB", (1, 1), (0, 0, 255)).convert("CMYK").getpixel((0, 0)) == (255, 255, 0, 0), "RGB to CMYK")
def cms():
    from PIL import ImageCms
    srgb = ImageCms.createProfile("sRGB"); lab = ImageCms.createProfile("LAB")
    t = ImageCms.buildTransform(srgb, lab, "RGB", "LAB")
    w = ImageCms.applyTransform(Image.new("RGB", (1, 1), (255, 255, 255)), t).getpixel((0, 0))
    check(w[0] == 255 and abs(w[1] - 128) <= 1 and abs(w[2] - 128) <= 1, "LittleCMS: sRGB white -> L*=100, a*=b*=0")
def text():
    from PIL import ImageFont
    font = None
    for p in ("/usr/local/lib/python3.14/site-packages/matplotlib/mpl-data/fonts/ttf/DejaVuSans.ttf",
              "/usr/local/share/fonts/font-dejavu/DejaVuSans.ttf", "/usr/local/share/fonts/TTF/DejaVuSans.ttf"):
        if os.path.exists(p): font = p; break
    if font is None:
        f = ImageFont.load_default(size=20)
        info("no DejaVu font file: Pillow's built-in font")
    else:
        f = ImageFont.truetype(font, 20, layout_engine=ImageFont.Layout.RAQM)
        info("font %s, layout RAQM" % font)
    im = Image.new("L", (120, 40), 0); ImageDraw.Draw(im).text((4, 4), "QNX", font=f, fill=255)
    bb = im.getbbox(); info("text bbox %s" % (bb,))
    check(bb is not None and bb[2] - bb[0] > 20, "text drawn")
    l1 = ImageDraw.Draw(im).textlength("QNX", font=f); l2 = ImageDraw.Draw(im).textlength("QNXQNX", font=f)
    check(l1 > 0 and abs(l2 - 2 * l1) < 1.5, "text length doubles with the string")
for n, fn in (("features", feats), ("draw", draw), ("formats", formats), ("ops", ops), ("cms", cms), ("text", text)):
    run(n, fn)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS python3-pillow"; exit 0; }
echo "FAIL python3-pillow (rc=$rc)"; exit 1
