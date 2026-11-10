#!/bin/sh
# python3-cairo (pycairo) on the ported cairo: an ARGB32 image surface gets
# an opaque red and a half transparent blue rectangle, and its pixels are
# exactly the premultiplied values; the image goes to PNG and back
# unchanged; PDF and SVG surfaces write real documents; a Matrix maps a
# point; a path's extents; the C API capsule PyGObject uses is there.
echo "== python3-cairo =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
# No __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183)
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pycairo.XXXXXX") || { echo "FAIL python3-cairo (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
cat > t.py <<'P'
import sys
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def info(s): print("  info " + s)
import cairo
info("pycairo %s, cairo %s" % (cairo.version, cairo.cairo_version_string()))
check(cairo.cairo_version_string() == "1.18.2", "linked against the ported cairo 1.18.2")
check(cairo._cairo.__file__.endswith(".so"), "cairo._cairo is the compiled extension")
s = cairo.ImageSurface(cairo.FORMAT_ARGB32, 4, 2)
cr = cairo.Context(s)
cr.set_source_rgb(1, 0, 0); cr.rectangle(0, 0, 2, 2); cr.fill()
cr.set_source_rgba(0, 0, 1, 0.5); cr.rectangle(2, 0, 2, 2); cr.fill()
s.flush()
check(s.get_stride() == 16 and (s.get_width(), s.get_height()) == (4, 2), "stride and size")
d = bytes(s.get_data())
px = lambda x, y: d[y * 16 + x * 4: y * 16 + x * 4 + 4]
# ARGB32 is native-endian 32-bit: B, G, R, A in memory on little endian
check(px(0, 0) == b"\x00\x00\xff\xff" and px(1, 1) == b"\x00\x00\xff\xff", "opaque red pixels")
check(px(3, 1) in (b"\x80\x00\x00\x80", b"\x7f\x00\x00\x7f"), "half transparent blue, premultiplied")
if px(3, 1) not in (b"\x80\x00\x00\x80", b"\x7f\x00\x00\x7f"): info(px(3, 1).hex())
s.write_to_png("o.png")
check(open("o.png", "rb").read(8) == b"\x89PNG\r\n\x1a\n", "write_to_png writes a PNG")
s2 = cairo.ImageSurface.create_from_png("o.png")
check(bytes(s2.get_data()) == d, "PNG read back has the same pixels")
p = cairo.PDFSurface("o.pdf", 200, 100); c = cairo.Context(p)
c.move_to(10, 10); c.line_to(190, 90); c.stroke(); c.show_page(); p.finish()
check(open("o.pdf", "rb").read(5) == b"%PDF-", "PDF surface writes a PDF")
v = cairo.SVGSurface("o.svg", 50, 50); c = cairo.Context(v)
c.arc(25, 25, 20, 0, 6.283185307179586); c.fill(); v.finish()
check(b"<svg" in open("o.svg", "rb").read(), "SVG surface writes SVG")
m = cairo.Matrix(xx=2, yy=3, x0=1, y0=1)
check(m.transform_point(1, 1) == (3.0, 4.0), "Matrix.transform_point")
mi = cairo.Matrix(xx=2, yy=3, x0=1, y0=1); mi.invert()
check(mi.transform_point(3, 4) == (1.0, 1.0), "Matrix.invert")
c = cairo.Context(cairo.ImageSurface(cairo.FORMAT_A8, 10, 10))
c.rectangle(1, 2, 3, 4)
check(c.path_extents() == (1.0, 2.0, 4.0, 6.0), "path extents")
check(hasattr(cairo, "CAPI"), "the C API capsule (cairo.CAPI) for PyGObject")
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
[ $rc -eq 0 ] && { echo "PASS python3-cairo"; exit 0; }
echo "FAIL python3-cairo (rc=$rc)"; exit 1
