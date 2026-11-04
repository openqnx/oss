#!/bin/sh
# Matplotlib with the Agg backend (no display): a figure rendered to PNG at
# a known size whose pixels are checked (Agg rasterizer, the _image module),
# text measured and drawn through ft2font (FreeType + libraqm), PDF/SVG/PS
# output, Delaunay triangulation (Qhull: the square with a centre point gives
# four triangles), contours (contourpy: the level of a known paraboloid),
# constrained layout (kiwisolver), property cycles (cycler) and a font read
# back with fontTools. Each of those packages is also used directly.
echo "== python3-matplotlib =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-mpl.XXXXXX") || { echo "FAIL python3-matplotlib (no temporary directory)"; exit 1; }
cd "$T" || exit 1
export MPLCONFIGDIR="$T/mplconfig" HOME="$T"
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
        import traceback; traceback.print_exc(limit=4, file=sys.stdout)
        check(False, "%s: %r" % (name, e))
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
info("matplotlib %s, backend %s" % (matplotlib.__version__, matplotlib.get_backend()))
def png():
    fig = plt.figure(figsize=(4, 3), dpi=50)
    ax = fig.add_axes([0, 0, 1, 1]); ax.set_axis_off()
    ax.add_patch(plt.Rectangle((0, 0), 0.5, 1, color=(1, 0, 0)))
    ax.add_patch(plt.Rectangle((0.5, 0), 0.5, 1, color=(0, 0, 1)))
    ax.set_xlim(0, 1); ax.set_ylim(0, 1)
    fig.savefig("p.png"); plt.close(fig)
    img = plt.imread("p.png")
    check(img.shape[:2] == (150, 200), "4x3 in at 50 dpi -> 200x150 PNG")
    check(np.allclose(img[75, 50, :3], [1, 0, 0]) and np.allclose(img[75, 150, :3], [0, 0, 1]), "left half red, right half blue")
    fig, ax = plt.subplots(figsize=(3, 2), dpi=80)
    ax.plot([0, 1, 2], [0, 1, 4], "g-"); ax.set_title("QNX")
    buf = io.BytesIO(); fig.savefig(buf, format="png"); plt.close(fig)
    check(buf.getvalue()[:8] == b"\x89PNG\r\n\x1a\n" and len(buf.getvalue()) > 1000, "line plot with title to PNG")
def text():
    from matplotlib.ft2font import FT2Font
    from matplotlib import font_manager
    path = font_manager.findfont("DejaVu Sans")
    check(os.path.basename(path) == "DejaVuSans.ttf", "bundled DejaVu Sans found")
    f = FT2Font(path); f.set_size(12, 72)
    f.set_text("QNX"); w1, h1 = f.get_width_height()
    f.set_text("QNX QNX"); w2, _ = f.get_width_height()
    info("'QNX' at 12 pt: %d x %d (1/64 px)" % (w1, h1))
    check(w1 > 0 and h1 > 0 and w2 > 2 * w1, "text extent grows with the string")
    fig = plt.figure(figsize=(1, 1), dpi=100); fig.text(0.5, 0.5, "Ag", size=40, ha="center", va="center")
    fig.canvas.draw(); a = np.asarray(fig.canvas.buffer_rgba()); plt.close(fig)
    check((a[..., :3] < 128).any(), "rendered text has dark pixels")
    r = matplotlib.mathtext.math_to_image(r"$\alpha^2+\sqrt{x}$", "m.png", dpi=72, format="png")
    check(os.path.getsize("m.png") > 100, "mathtext rendered")
def vector():
    fig, ax = plt.subplots(); ax.plot([1, 2, 3])
    for fmt, magic in (("pdf", b"%PDF-"), ("svg", b"<?xml"), ("ps", b"%!PS"), ("eps", b"%!PS")):
        b = io.BytesIO(); fig.savefig(b, format=fmt)
        check(b.getvalue().startswith(magic), "%s output" % fmt)
    plt.close(fig)
def tri():
    import matplotlib.tri as mtri
    t = mtri.Triangulation([0, 1, 1, 0, 0.5], [0, 0, 1, 1, 0.5])
    check(len(t.triangles) == 4 and len(t.edges) == 8, "Delaunay of a square + centre: 4 triangles, 8 edges (Qhull)")
    f = mtri.LinearTriInterpolator(t, np.array([0, 1, 2, 1, 1.0]))
    check(abs(float(f(0.5, 0.5)) - 1.0) < 1e-12 and abs(float(f(0.75, 0.5)) - 1.25) < 1e-12, "linear interpolation on the triangulation")
def contour():
    import contourpy
    x, y = np.meshgrid(np.linspace(-1, 1, 101), np.linspace(-1, 1, 101)); z = x ** 2 + y ** 2
    gen = contourpy.contour_generator(x, y, z)
    lines = gen.lines(0.25)
    check(len(lines) == 1, "contourpy: one closed line at z = 0.25")
    r = np.hypot(lines[0][:, 0], lines[0][:, 1])
    check(np.allclose(r, 0.5, atol=2e-3), "contourpy: the line is the circle r = 0.5")
    filled = gen.filled(0.0, 0.25)
    check(len(filled[0]) == 1, "contourpy: filled contour 0..0.25")
    fig, ax = plt.subplots(); cs = ax.contour(x, y, z, levels=[0.25, 0.5]); plt.close(fig)
    check(list(cs.levels) == [0.25, 0.5], "ax.contour levels")
def layout():
    import kiwisolver as kiwi
    a, b = kiwi.Variable("a"), kiwi.Variable("b")
    s = kiwi.Solver(); s.addConstraint(a + b == 10); s.addConstraint(a - b == 4); s.updateVariables()
    check((a.value(), b.value()) == (7.0, 3.0), "kiwisolver: a+b=10, a-b=4 -> 7, 3")
    s2 = kiwi.Solver(); x = kiwi.Variable("x"); s2.addConstraint(x >= 5); s2.addConstraint((x == 2) | "weak"); s2.updateVariables()
    check(x.value() == 5.0, "kiwisolver: weak x == 2 loses to required x >= 5")
    fig, axs = plt.subplots(2, 2, layout="constrained", figsize=(4, 4))
    for ax in axs.flat: ax.set_title("t"); ax.set_ylabel("y")
    fig.canvas.draw()
    bb = [ax.get_position() for ax in axs.flat]; plt.close(fig)
    check(bb[0].x1 < bb[1].x0 and bb[2].y1 < bb[0].y0, "constrained layout separates the axes")
def cyc():
    from cycler import cycler
    c = cycler(color=["r", "g"]) * cycler(ls=["-", ":"])
    check(len(c) == 4 and list(c)[1] == {"color": "r", "ls": ":"}, "cycler product")
    fig, ax = plt.subplots(); ax.set_prop_cycle(cycler(color=["#123456", "#abcdef"]))
    l1, = ax.plot([0, 1]); l2, = ax.plot([1, 0]); plt.close(fig)
    check((l1.get_color(), l2.get_color()) == ("#123456", "#abcdef"), "axes property cycle")
def fonts():
    from fontTools.ttLib import TTFont
    from matplotlib import font_manager
    f = TTFont(font_manager.findfont("DejaVu Sans"))
    check(f["head"].unitsPerEm == 2048 and f["name"].getDebugName(1) == "DejaVu Sans", "fontTools: DejaVu Sans, 2048 units/em")
    check(f.getBestCmap()[ord("A")] == "A" and f["hmtx"]["A"][0] > 0, "fontTools: cmap and advance of 'A'")
def dates():
    import datetime as dt, matplotlib.dates as mdates
    d = mdates.date2num(dt.datetime(1970, 1, 2))
    check(d == 1.0 and mdates.num2date(1.5).hour == 12, "matplotlib.dates epoch")
for n, f in (("png", png), ("text", text), ("vector", vector), ("tri", tri), ("contour", contour),
             ("layout", layout), ("cycler", cyc), ("fonttools", fonts), ("dates", dates)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS python3-matplotlib"; exit 0; }
echo "FAIL python3-matplotlib (rc=$rc)"; exit 1
