#!/bin/sh
# pygame on SDL2's dummy video and audio drivers (the target has no display):
# a display surface, drawing with known pixels, blits, transforms and masks,
# images saved and loaded through SDL_image (PNG, JPEG, BMP, TGA), text
# rendered with SDL_ttf (pygame.font) and FreeType (pygame.freetype) from
# the bundled freesansbold.ttf, the mixer on SDL_mixer (a generated tone as a
# Sound, its length, a WAV written by the test and loaded back), surfarray
# and sndarray on numpy, events, Rect and math.
echo "== pygame =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pygame.XXXXXX") || { echo "FAIL pygame (no temporary directory)"; exit 1; }
cd "$T" || exit 1
export SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy PYGAME_HIDE_SUPPORT_PROMPT=1 HOME="$T"
cat > t.py <<'P'
import sys, os, math, struct, wave
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
import pygame, pygame.freetype, pygame.sndarray, pygame.surfarray
import numpy as np
info("pygame %s, SDL %s" % (pygame.version.ver, ".".join(map(str, pygame.get_sdl_version()))))
passed, failed = pygame.init()
info("pygame.init(): %d modules up, %d failed" % (passed, failed))
def display():
    s = pygame.display.set_mode((64, 48))
    check(pygame.display.get_driver() == "dummy" and s.get_size() == (64, 48), "display.set_mode on the dummy driver: 64x48")
    s.fill((0, 0, 0)); pygame.draw.rect(s, (255, 0, 0), (8, 8, 16, 16)); pygame.display.flip()
    check(s.get_at((10, 10))[:3] == (255, 0, 0) and s.get_at((40, 40))[:3] == (0, 0, 0), "drawn rect on the display surface")
    pygame.display.set_caption("qnx-pygame"); check(pygame.display.get_caption()[0] == "qnx-pygame", "window caption")
def draw():
    s = pygame.Surface((40, 30))
    s.fill((255, 255, 255))
    pygame.draw.circle(s, (0, 0, 255), (20, 15), 10)
    pygame.draw.line(s, (0, 255, 0), (0, 29), (39, 29))
    check(s.get_at((20, 15)) == pygame.Color(0, 0, 255) and s.get_at((1, 1)) == pygame.Color(255, 255, 255), "circle: centre blue, corner white")
    check(s.get_at((5, 29)) == pygame.Color(0, 255, 0), "line on the last row")
    d = pygame.Surface((80, 60)); d.fill((0, 0, 0)); d.blit(s, (40, 30))
    check(d.get_at((60, 45))[:3] == (0, 0, 255) and d.get_at((10, 10))[:3] == (0, 0, 0), "blit at an offset")
    big = pygame.transform.scale(s, (80, 60)); check(big.get_size() == (80, 60) and big.get_at((40, 30))[:3] == (0, 0, 255), "transform.scale")
    rot = pygame.transform.rotate(pygame.Surface((40, 20)), 90); check(rot.get_size() == (20, 40), "transform.rotate 90: 40x20 -> 20x40")
    fl = pygame.transform.flip(s, True, False); check(fl.get_at((39 - 5, 29))[:3] == (0, 255, 0), "transform.flip")
    m = pygame.mask.from_threshold(s, (0, 0, 255), (1, 1, 1, 255)); n = m.count()
    check(abs(n - math.pi * 100) < 40, "mask of the blue circle: %d pixels (~pi*10^2)" % n)
def images():
    s = pygame.Surface((32, 24)); s.fill((10, 200, 30)); s.fill((250, 0, 0), (0, 0, 16, 24))
    for ext in ("png", "bmp", "tga", "jpg"):
        pygame.image.save(s, "i." + ext)
        b = pygame.image.load("i." + ext)
        if ext == "jpg":
            r, g, bb, _ = b.get_at((4, 4)); check(b.get_size() == (32, 24) and r > 200 and g < 50, "JPEG save/load (SDL_image) close to the original")
        else:
            check(b.get_size() == (32, 24) and b.get_at((4, 4))[:3] == (250, 0, 0) and b.get_at((28, 4))[:3] == (10, 200, 30), "%s save/load exact" % ext.upper())
    check(pygame.image.get_extended(), "image.get_extended(): SDL_image formats available")
    raw = pygame.image.tobytes(s, "RGB"); check(len(raw) == 32 * 24 * 3 and raw[:3] == bytes((250, 0, 0)), "image.tobytes RGB")
def fonts():
    pygame.font.init()
    f = pygame.font.Font(None, 24)
    w, h = f.size("QNX"); w2, _ = f.size("QNXQNX")
    info("font.size('QNX') = %dx%d" % (w, h))
    check(w > 10 and h > 10 and abs(w2 - 2 * w) <= 2, "pygame.font (SDL_ttf): text size")
    t = f.render("QNX", True, (255, 255, 255))
    check(t.get_width() == w and abs(t.get_height() - h) <= 2 and t.get_bounding_rect().width > 10, "pygame.font render (anti-aliased, with alpha)")
    pygame.freetype.init()
    ft = pygame.freetype.Font(None, 20)
    surf, rect = ft.render("QNX", (255, 255, 0))
    check(rect.width > 10 and surf.get_size() == rect.size, "pygame.freetype render")
def mixer():
    pygame.mixer.quit()   # pygame.init() opened it with the defaults
    pygame.mixer.init(frequency=22050, size=-16, channels=1, buffer=512)
    q = pygame.mixer.get_init(); info("mixer.get_init() = %s" % (q,))
    check(q is not None and q[0] == 22050 and q[2] == 1, "mixer on the dummy audio driver: 22050 Hz mono")
    n = 22050 // 2
    data = b"".join(struct.pack("<h", int(8000 * math.sin(2 * math.pi * 440 * i / 22050))) for i in range(n))
    snd = pygame.mixer.Sound(buffer=data)
    check(abs(snd.get_length() - 0.5) < 0.01 and snd.get_raw() == data, "Sound from a buffer: 0.5 s, raw samples kept")
    ch = snd.play(); check(ch is not None and pygame.mixer.get_busy(), "Sound.play() on a channel")
    pygame.mixer.stop()
    with wave.open("t.wav", "wb") as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(22050); w.writeframes(data)
    s2 = pygame.mixer.Sound("t.wav"); check(abs(s2.get_length() - 0.5) < 0.01, "WAV file loaded by SDL_mixer: 0.5 s")
    pygame.mixer.music.load("t.wav"); pygame.mixer.music.play(); check(pygame.mixer.music.get_busy(), "mixer.music plays the WAV"); pygame.mixer.music.stop()
    a = pygame.sndarray.array(snd); check(a.shape[0] == n and int(a.max()) <= 8000 and int(a.max()) > 7900, "sndarray: samples as numpy")
    pygame.mixer.quit()
def arrays():
    s = pygame.Surface((4, 3)); s.fill((1, 2, 3))
    a = pygame.surfarray.array3d(s); check(a.shape == (4, 3, 3) and a[0, 0].tolist() == [1, 2, 3], "surfarray.array3d")
    b = np.zeros((5, 6, 3), dtype=np.uint8); b[..., 0] = 200
    t = pygame.surfarray.make_surface(b); check(t.get_size() == (5, 6) and t.get_at((2, 2))[:3] == (200, 0, 0), "surfarray.make_surface")
def misc():
    r = pygame.Rect(0, 0, 10, 10)
    check(r.colliderect((5, 5, 10, 10)) and r.clip((5, 5, 10, 10)) == pygame.Rect(5, 5, 5, 5) and r.union((20, 0, 1, 1)).width == 21, "Rect collide, clip, union")
    v = pygame.math.Vector2(3, 4); check(v.length() == 5 and v.normalize() * 5 == v and pygame.math.Vector3(1, 0, 0).cross((0, 1, 0)) == (0, 0, 1), "math.Vector2/3")
    pygame.event.clear(); pygame.event.post(pygame.event.Event(pygame.USEREVENT, code=42))
    ev = [e for e in pygame.event.get() if e.type == pygame.USEREVENT]
    check(len(ev) == 1 and ev[0].code == 42, "event post/get")
    c = pygame.time.Clock(); t0 = pygame.time.get_ticks(); pygame.time.wait(50); c.tick()
    check(pygame.time.get_ticks() - t0 >= 45, "time.wait(50)")
    check(pygame.Color("red") == (255, 0, 0, 255) and pygame.Color(0, 0, 255).hsva[0] == 240, "Color names, HSV")
for n, f in (("display", display), ("draw", draw), ("images", images), ("fonts", fonts), ("mixer", mixer), ("arrays", arrays), ("misc", misc)):
    run(n, f)
pygame.quit()
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS pygame"; exit 0; }
echo "FAIL pygame (rc=$rc)"; exit 1
